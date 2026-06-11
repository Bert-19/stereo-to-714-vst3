#pragma once

#include "UpmixConfig.h"
#include <array>
#include <complex>
#include <vector>

namespace upmix
{

class Decorrelator
{
public:
    void prepare (double sampleRate);
    void reset();

    std::complex<float> processBin (int channelIndex, int binIndex, std::complex<float> value) const;

private:
    std::array<std::vector<float>, kNum714Channels> binPhase;
};

} // namespace upmix
