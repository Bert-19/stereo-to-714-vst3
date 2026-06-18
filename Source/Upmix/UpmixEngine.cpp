#include "UpmixEngine.h"
#include <cmath>

namespace upmix
{

namespace
{
constexpr float kOlaNorm = 2.0f / 3.0f;
}

UpmixEngine::UpmixEngine()
    : fft (std::make_unique<juce::dsp::FFT> (kFftOrder))
{
}

void UpmixEngine::prepare (double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;
    spatial.prepare (sampleRate);
    decorrelator.prepare (sampleRate);

    window.resize (static_cast<size_t> (kFftSize));
    for (int i = 0; i < kFftSize; ++i)
        window[static_cast<size_t> (i)] = 0.5f * (1.0f - std::cos (2.0f * juce::MathConstants<float>::pi * static_cast<float> (i)
                                                                                      / static_cast<float> (kFftSize)));

    const auto fftBufferSize = static_cast<size_t> (kFftSize * 2);
    fftBufferL.assign (fftBufferSize, 0.0f);
    fftBufferR.assign (fftBufferSize, 0.0f);

    for (auto& buffer : fftBuffersOut)
        buffer.assign (fftBufferSize, 0.0f);

    const auto fifoSize = static_cast<size_t> (kFftSize * 4 + maxBlockSize);
    inputFifoL.assign (fifoSize, 0.0f);
    inputFifoR.assign (fifoSize, 0.0f);

    for (auto& fifo : outputFifo)
        fifo.assign (fifoSize, 0.0f);

    reset();
    prepared = true;
}

void UpmixEngine::reset()
{
    std::fill (inputFifoL.begin(), inputFifoL.end(), 0.0f);
    std::fill (inputFifoR.begin(), inputFifoR.end(), 0.0f);

    for (auto& fifo : outputFifo)
        std::fill (fifo.begin(), fifo.end(), 0.0f);

    inputWritePos = 0;
    outputReadPos = 0;
    outputWritePos = kFftSize - kHopSize;
    samplesUntilHop = kHopSize;
}

void UpmixEngine::setParams (const UpmixParams& newParams)
{
    params = newParams;
}

std::complex<float> UpmixEngine::readSpectrumBin (const float* data, int binIndex)
{
    if (binIndex == 0)
        return { data[0], 0.0f };

    if (binIndex == kNumBins - 1)
        return { data[1], 0.0f };

    return { data[binIndex * 2], data[binIndex * 2 + 1] };
}

void UpmixEngine::writeSpectrumBin (float* data, int binIndex, std::complex<float> value)
{
    if (binIndex == 0)
    {
        data[0] = value.real();
        return;
    }

    if (binIndex == kNumBins - 1)
    {
        data[1] = value.real();
        return;
    }

    data[binIndex * 2] = value.real();
    data[binIndex * 2 + 1] = value.imag();
}

float UpmixEngine::lfeWeight (int binIndex) const
{
    const float freq = static_cast<float> (binIndex * sampleRate / static_cast<double> (kFftSize));
    const float ratio = freq / kLfeCutoffHz;
    if (ratio >= 2.0f)
        return 0.0f;

    if (ratio <= 0.5f)
        return 1.0f;

    return std::exp (-std::pow ((ratio - 0.5f) / 0.6f, 2.0f));
}

void UpmixEngine::processSpectrumBin (int binIndex,
                                      std::complex<float> xl,
                                      std::complex<float> xr,
                                      std::array<std::complex<float>, kNum714Channels>& outputs)
{
    outputs.fill ({ 0.0f, 0.0f });

    float theta = 0.0f;
    float psi = 0.0f;
    float gamma = 0.0f;
    spatial.analyseBin (binIndex, xl, xr, theta, psi, gamma);

    const auto mid = (xl + xr) * 0.5f;
    const auto side = (xl - xr) * 0.5f;

    const float gammaPow = params.centerMode == CenterMode::movie ? 1.5f : 0.5f;
    const float centerGain = centerWeight (psi, theta, params.centerMode) * std::pow (gamma, gammaPow);

    float frontL = directWeight (psi, theta, +30.0f, 28.0f);
    float frontR = directWeight (psi, theta, -30.0f, 28.0f);

    if (params.centerMode == CenterMode::movie)
    {
        const float duck = 1.0f - 0.85f * centerGain;
        frontL *= duck;
        frontR *= duck;
    }

    outputs[static_cast<size_t> (chL)] = xl * frontL;
    outputs[static_cast<size_t> (chR)] = xr * frontR;
    outputs[static_cast<size_t> (chC)] = mid * centerGain;

    const float sideDirectL = directWeight (psi, theta, +110.0f, 35.0f) * params.width;
    const float sideDirectR = directWeight (psi, theta, -110.0f, 35.0f) * params.width;
    outputs[static_cast<size_t> (chLs)] += side * sideDirectL;
    outputs[static_cast<size_t> (chRs)] -= side * sideDirectR;

    const float rearL = angularGaussian (theta, +150.0f, 40.0f) * (1.0f - psi) * params.width;
    const float rearR = angularGaussian (theta, -150.0f, 40.0f) * (1.0f - psi) * params.width;
    outputs[static_cast<size_t> (chLrs)] += xl * rearL - xr * rearR * 0.65f;
    outputs[static_cast<size_t> (chRrs)] += xr * rearR - xl * rearL * 0.65f;

    const float ambient = psi * params.surroundMix * params.width;
    const auto ambientLeft = side * ambient;
    const auto ambientRight = -side * ambient;

    outputs[static_cast<size_t> (chLs)] += ambientLeft * 0.55f;
    outputs[static_cast<size_t> (chRs)] += ambientRight * 0.55f;
    outputs[static_cast<size_t> (chLrs)] += ambientLeft * 0.45f;
    outputs[static_cast<size_t> (chRrs)] += ambientRight * 0.45f;

    const float freq = static_cast<float> (binIndex * sampleRate / static_cast<double> (kFftSize));
    float skyMix = skyGate (psi, params.heightMode) * params.heightAmount;

    if (params.heightMode == HeightMode::aggressive)
        skyMix *= 0.65f + 0.35f * highFrequencyWeight (freq, static_cast<float> (sampleRate));

    const auto skySignal = side * skyMix;
    outputs[static_cast<size_t> (chLtf)] = skySignal * skyDirectionGain (theta, +45.0f);
    outputs[static_cast<size_t> (chRtf)] = -skySignal * skyDirectionGain (theta, -45.0f);
    outputs[static_cast<size_t> (chLtr)] = skySignal * skyDirectionGain (theta, +135.0f);
    outputs[static_cast<size_t> (chRtr)] = -skySignal * skyDirectionGain (theta, -135.0f);

    if (params.heightMode == HeightMode::aggressive)
    {
        const float hf = highFrequencyWeight (freq, static_cast<float> (sampleRate)) * (1.0f - psi) * 0.25f;
        outputs[static_cast<size_t> (chLtf)] += mid * hf * skyDirectionGain (theta, +45.0f);
        outputs[static_cast<size_t> (chRtf)] += mid * hf * skyDirectionGain (theta, -45.0f);
    }

    outputs[static_cast<size_t> (chLFE)] = mid * lfeWeight (binIndex) * params.lfeLevel;

    for (int ch : { chLs, chRs, chLrs, chRrs, chLtf, chRtf, chLtr, chRtr })
        outputs[static_cast<size_t> (ch)] = decorrelator.processBin (ch, binIndex, outputs[static_cast<size_t> (ch)]);

    const float inEnergy = std::norm (xl) + std::norm (xr) + 1.0e-8f;
    float outEnergy = 0.0f;

    for (const auto& sample : outputs)
        outEnergy += std::norm (sample);

    const float maxGain = 1.35f;
    if (outEnergy > inEnergy * maxGain)
    {
        const float scale = std::sqrt ((inEnergy * maxGain) / outEnergy);

        for (auto& sample : outputs)
            sample *= scale;
    }
}

void UpmixEngine::processHop()
{
    const auto fifoSize = static_cast<int> (inputFifoL.size());

    for (int i = 0; i < kFftSize; ++i)
    {
        const auto index = (inputWritePos - kFftSize + i + fifoSize) % fifoSize;
        const auto w = window[static_cast<size_t> (i)];

        fftBufferL[static_cast<size_t> (i)] = inputFifoL[static_cast<size_t> (index)] * w;
        fftBufferR[static_cast<size_t> (i)] = inputFifoR[static_cast<size_t> (index)] * w;
    }

    std::fill (fftBufferL.begin() + kFftSize, fftBufferL.end(), 0.0f);
    std::fill (fftBufferR.begin() + kFftSize, fftBufferR.end(), 0.0f);

    fft->performRealOnlyForwardTransform (fftBufferL.data());
    fft->performRealOnlyForwardTransform (fftBufferR.data());

    std::array<std::complex<float>, kNum714Channels> spectrumOut {};

    for (int bin = 0; bin < kNumBins; ++bin)
    {
        const auto xl = readSpectrumBin (fftBufferL.data(), bin);
        const auto xr = readSpectrumBin (fftBufferR.data(), bin);
        processSpectrumBin (bin, xl, xr, spectrumOut);

        for (int ch = 0; ch < kNum714Channels; ++ch)
            writeSpectrumBin (fftBuffersOut[static_cast<size_t> (ch)].data(), bin, spectrumOut[static_cast<size_t> (ch)]);
    }

    for (int ch = 0; ch < kNum714Channels; ++ch)
    {
        auto& buffer = fftBuffersOut[static_cast<size_t> (ch)];
        fft->performRealOnlyInverseTransform (buffer.data());

        for (int i = 0; i < kFftSize; ++i)
        {
            const auto sample = buffer[static_cast<size_t> (i)] * window[static_cast<size_t> (i)] * kOlaNorm;
            const auto pos = (outputWritePos + i) % fifoSize;
            outputFifo[static_cast<size_t> (ch)][static_cast<size_t> (pos)] += sample;
        }
    }

    outputWritePos = (outputWritePos + kHopSize) % fifoSize;
}

void UpmixEngine::process (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output)
{
    const auto numSamples = input.getNumSamples();
    const auto numOutChannels = juce::jmin (output.getNumChannels(), kNum714Channels);

    if (params.bypass)
    {
        for (int ch = 0; ch < numOutChannels; ++ch)
            output.clear (ch, 0, numSamples);

        if (numOutChannels > 0)
            output.copyFrom (0, 0, input, 0, 0, numSamples);

        if (numOutChannels > 1)
            output.copyFrom (1, 0, input, 1, 0, numSamples);

        return;
    }

    if (! prepared)
    {
        output.clear();
        return;
    }

    const auto* inL = input.getReadPointer (0);
    const auto* inR = input.getReadPointer (1);
    const auto fifoSize = static_cast<int> (inputFifoL.size());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        inputFifoL[static_cast<size_t> (inputWritePos)] = inL[sample];
        inputFifoR[static_cast<size_t> (inputWritePos)] = inR[sample];
        inputWritePos = (inputWritePos + 1) % fifoSize;

        --samplesUntilHop;

        if (samplesUntilHop == 0)
        {
            processHop();
            samplesUntilHop = kHopSize;
        }

        for (int ch = 0; ch < numOutChannels; ++ch)
        {
            auto* out = output.getWritePointer (ch);
            out[sample] = outputFifo[static_cast<size_t> (ch)][static_cast<size_t> (outputReadPos)];
            outputFifo[static_cast<size_t> (ch)][static_cast<size_t> (outputReadPos)] = 0.0f;
        }

        outputReadPos = (outputReadPos + 1) % fifoSize;
    }
}

} // namespace upmix
