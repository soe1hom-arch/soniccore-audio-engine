#include "soniccore/sonic_core.h"

#include <cassert>
#include <cmath>
#include <vector>

static float rms(const float* data, int count) {
    float sum = 0.0f;
    for (int i = 0; i < count; ++i) sum += data[i] * data[i];
    return std::sqrt(sum / static_cast<float>(count));
}

int main() {
    soniccore::Config config;
    config.limiter_enabled = true;
    config.limiter_ceiling = 0.98f;
    soniccore::Engine engine(config);

    float samples[] = {1.5f, -1.5f, 0.1f, -0.1f};
    engine.process(samples, 2);
    assert(std::abs(samples[0]) <= 1.0f);
    assert(std::abs(samples[1]) <= 1.0f);

    engine.set_bypassed(true);
    float bypassed[] = {1.2f, -1.2f};
    engine.process(bypassed, 1);
    assert(bypassed[0] == 1.2f);
    assert(bypassed[1] == -1.2f);

    soniccore::Config flat_config;
    flat_config.headroom_db = 0.0f;
    flat_config.limiter_enabled = false;
    soniccore::Engine flat(flat_config);
    float signal[] = {0.1f, -0.1f, 0.2f, -0.2f};
    const float before = rms(signal, 4);
    flat.process(signal, 2);
    assert(std::abs(rms(signal, 4) - before) < 0.001f);

    soniccore::Parameters tuned;
    tuned.eq_gain_db[5] = 6.0f;
    tuned.bass_db = 3.0f;
    tuned.treble_db = -2.0f;
    flat.set_parameters(tuned);
    flat.reset();
    float shaped[] = {0.1f, -0.1f, 0.2f, -0.2f};
    flat.process(shaped, 2);
    assert(std::isfinite(shaped[0]));

    soniccore::Config spatial_config;
    spatial_config.eq_enabled = false;
    spatial_config.tone_enabled = false;
    spatial_config.spatial_enabled = true;
    spatial_config.reverb_enabled = true;
    spatial_config.headroom_db = 0.0f;
    spatial_config.limiter_enabled = true;
    soniccore::Engine spatial(spatial_config);
    soniccore::Parameters spatial_params;
    spatial_params.spatial_mix = 1.0f;
    spatial_params.spatial_width = 0.35f;
    spatial_params.spatial_pan_depth = 0.4f;
    spatial_params.spatial_rotate = true;
    spatial_params.reverb_mix = 0.25f;
    spatial_params.reverb_room = 0.5f;
    spatial.set_parameters(spatial_params);
    spatial.set_effects_enabled(false, false, true, true, true);
    std::vector<float> stereo(2048, 0.0f);
    stereo[0] = 0.5f;
    stereo[1] = 0.5f;
    spatial.process(stereo.data(), stereo.size() / 2);
    for (float sample : stereo) assert(std::isfinite(sample));
    assert(std::abs(stereo[0]) <= 1.0f);
    assert(std::abs(stereo[1]) <= 1.0f);
    return 0;
}
