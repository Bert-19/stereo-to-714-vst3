#pragma once

#include "UpmixConfig.h"
#include <complex>
#include <vector>

namespace upmix
{

class SpatialEstimator
{
public:
    void prepare (double sampleRate);
    void reset();

    void analyseBin (int binIndex,
                     std::complex<float> xl,
                     std::complex<float> xr,
                     float& thetaDeg,
                     float& psi,
                     float& gamma) const;

private:
    double sampleRate = 48000.0;
    std::vector<float> ildToTheta;
    std::vector<float> ipdToTheta;
    std::vector<float> ildWeight;
    std::vector<float> ipdWeight;

    static float binFrequency (int binIndex, double sr);
};

} // namespace upmix
