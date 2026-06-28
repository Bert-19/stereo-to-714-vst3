#include "Upmix/UpmixConfig.h"

int main()
{
    static_assert (upmix::kNum714Channels == 12);

    static_assert (upmix::chL == 0);
    static_assert (upmix::chR == 1);
    static_assert (upmix::chC == 2);
    static_assert (upmix::chLs == 3);
    static_assert (upmix::chRs == 4);
    static_assert (upmix::chLrs == 5);
    static_assert (upmix::chRrs == 6);
    static_assert (upmix::chLFE == 7);
    static_assert (upmix::chLtf == 8);
    static_assert (upmix::chRtf == 9);
    static_assert (upmix::chLtr == 10);
    static_assert (upmix::chRtr == 11);

    static_assert (upmix::kSpeakers[upmix::chLs].azimuthDeg == +110.0f);
    static_assert (upmix::kSpeakers[upmix::chRs].azimuthDeg == -110.0f);
    static_assert (upmix::kSpeakers[upmix::chLrs].azimuthDeg == +150.0f);
    static_assert (upmix::kSpeakers[upmix::chRrs].azimuthDeg == -150.0f);
    static_assert (upmix::kSpeakers[upmix::chLFE].azimuthDeg == 0.0f);
    static_assert (! upmix::kSpeakers[upmix::chLFE].isHeight);

    return 0;
}
