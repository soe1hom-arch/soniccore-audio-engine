#pragma once

#include "soniccore/sonic_core.h"

namespace soniccore {

enum class Profile {
    Clean,
    Music,
    Bass,
    Spatial,
    Room
};

struct ProfileSettings {
    Config config;
    Parameters parameters;
};

// Returns conservative starting points. Profiles never disable the safety
// limiter and can be further tuned by the host application or ROM layer.
ProfileSettings make_profile(Profile profile, int sample_rate = 48000, int channels = 2);

}  // namespace soniccore

