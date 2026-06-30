#include <JuceHeader.h>

#include <iostream>

#include "../Source/PluginProcessor.h"

namespace
{
int fail (const char* message)
{
    std::cerr << message << '\n';
    return 1;
}
} // namespace

int main()
{
    StereoTo714AudioProcessor processor;

    auto* bypassParameter = processor.getBypassParameter();
    if (bypassParameter == nullptr)
        return fail ("Host bypass parameter is not exposed");

    auto* apvtsBypassParameter = processor.getApvts().getParameter ("bypass");
    if (bypassParameter != apvtsBypassParameter)
        return fail ("Host bypass is not wired to the APVTS bypass parameter");

    bypassParameter->setValueNotifyingHost (1.0f);
    const auto* bypassValue = processor.getApvts().getRawParameterValue ("bypass");
    if (bypassValue == nullptr || *bypassValue < 0.5f)
        return fail ("Host bypass changes do not update processing bypass state");

    return 0;
}
