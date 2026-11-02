#include "soniccore/profiles.h"

namespace soniccore {

ProfileSettings make_profile(Profile profile, int sample_rate, int channels) {
    ProfileSettings settings;
    settings.config.sample_rate = sample_rate;
    settings.config.channels = channels;
    settings.config.headroom_db = -1.5f;
    settings.config.limiter_enabled = true;

    switch (profile) {
        case Profile::Clean:
            break;
        case Profile::Music:
            settings.parameters.eq_gain_db[3] = 1.0f;
            settings.parameters.eq_gain_db[5] = 0.75f;
            settings.parameters.eq_gain_db[7] = 1.0f;
            break;
        case Profile::Bass:
            settings.parameters.bass_db = 3.0f;
            settings.parameters.eq_gain_db[0] = 1.5f;
            settings.parameters.eq_gain_db[1] = 1.0f;
            break;
        case Profile::Spatial:
            settings.config.spatial_enabled = true;
            settings.parameters.spatial_mix = 1.0f;
            settings.parameters.spatial_rotate = true;
            settings.parameters.spatial_width = 0.35f;
            settings.parameters.spatial_pan_depth = 0.4f;
            break;
        case Profile::Room:
            settings.config.spatial_enabled = true;
            settings.config.reverb_enabled = true;
            settings.parameters.spatial_mix = 0.35f;
            settings.parameters.spatial_width = 0.25f;
            settings.parameters.reverb_mix = 0.12f;
            settings.parameters.reverb_room = 0.45f;
            break;
    }
    return settings;
}

}  // namespace soniccore

