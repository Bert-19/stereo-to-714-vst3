#pragma once

#include <JuceHeader.h>
#include "Decorrelator.h"
#include "GainFunctions.h"
#include "SpatialEstimator.h"
#include <array>
#include <complex>
#include <vector>

namespace upmix
{

class UpmixEngine
{
public:
    UpmixEngine();
    ~UpmixEngine() = default;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void setParams (const UpmixParams& newParams);

    int getLatencySamples() const noexcept { return kFftSize - kHopSize; }

    void process (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output);

private:
    static std::complex<float> readSpectrumBin (const float* data, int binIndex);
    static void writeSpectrumBin (float* data, int binIndex, std::complex<float> value);

    void processHop();
    void processSpectrumBin (int binIndex,
                             std::complex<float> xl,
                             std::complex<float> xr,
                             std::array<std::complex<float>, kNum714Channels>& outputs);

    float lfeWeight (int binIndex) const;

    SpatialEstimator spatial;
    Decorrelator decorrelator;

    std::unique_ptr<juce::dsp::FFT> fft;
    UpmixParams params;

    std::vector<float> window;
    std::vector<float> fftBufferL;
    std::vector<float> fftBufferR;
    std::array<std::vector<float>, kNum714Channels> fftBuffersOut;

    std::vector<float> inputFifoL;
    std::vector<float> inputFifoR;
    std::array<std::vector<float>, kNum714Channels> outputFifo;

    int inputWritePos = 0;
    int outputReadPos = 0;
    int outputWritePos = 0;
    int samplesUntilHop = kHopSize;

    double sampleRate = 48000.0;
    bool prepared = false;
};

} // namespace upmix
