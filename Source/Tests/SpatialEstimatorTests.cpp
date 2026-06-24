#include "../Upmix/SpatialEstimator.h"

#include <cmath>
#include <complex>
#include <iostream>
#include <string>

namespace
{

bool expect (bool condition, const std::string& message)
{
    if (! condition)
        std::cerr << "FAIL: " << message << '\n';

    return condition;
}

bool hardPannedBinsStayDirectional()
{
    upmix::SpatialEstimator estimator;
    estimator.prepare (48000.0);

    float theta = 0.0f;
    float psi = 0.0f;
    float gamma = 0.0f;
    bool passed = true;

    estimator.analyseBin (128, { 1.0f, 0.0f }, { 0.0f, 0.0f }, theta, psi, gamma);
    passed &= expect (gamma > 0.99f, "left-only bin was not treated as directional");
    passed &= expect (psi < 0.01f, "left-only bin was treated as diffuse");
    passed &= expect (theta > 100.0f, "left-only bin did not use leftward ILD");

    estimator.analyseBin (128, { 0.0f, 0.0f }, { 1.0f, 0.0f }, theta, psi, gamma);
    passed &= expect (gamma > 0.99f, "right-only bin was not treated as directional");
    passed &= expect (psi < 0.01f, "right-only bin was treated as diffuse");
    passed &= expect (theta < -100.0f, "right-only bin did not use rightward ILD");

    return passed;
}

} // namespace

int main()
{
    if (! hardPannedBinsStayDirectional())
        return 1;

    std::cout << "SpatialEstimator tests passed\n";
    return 0;
}
