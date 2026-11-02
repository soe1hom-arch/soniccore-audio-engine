#include "soniccore/sonic_core.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace {
constexpr float kPi = 3.14159265358979323846f;

float rms(const std::vector<float>& samples) {
    double sum = 0.0;
    for (float sample : samples) sum += static_cast<double>(sample) * sample;
    return static_cast<float>(std::sqrt(sum / samples.size()));
}

float peak(const std::vector<float>& samples) {
    float result = 0.0f;
    for (float sample : samples) result = std::max(result, std::abs(sample));
    return result;
}

std::vector<float> sine(int sample_rate, float frequency, int frames, float amplitude) {
    std::vector<float> samples(static_cast<std::size_t>(frames * 2));
    for (int frame = 0; frame < frames; ++frame) {
        const float value = amplitude * std::sin(2.0f * kPi * frequency * frame / sample_rate);
        samples[static_cast<std::size_t>(frame * 2)] = value;
        samples[static_cast<std::size_t>(frame * 2 + 1)] = value;
    }
    return samples;
}
}

int main() {
    for (int sample_rate : {44100, 48000}) {
        soniccore::Config base_config;
        base_config.sample_rate = sample_rate;
        base_config.headroom_db = 0.0f;
        base_config.limiter_enabled = false;

        soniccore::Engine flat(base_config);
        auto reference = sine(sample_rate, 1000.0f, 4096, 0.2f);
        auto flat_signal = reference;
        flat.process(flat_signal.data(), flat_signal.size() / 2);
        assert(std::abs(rms(flat_signal) - rms(reference)) < 0.002f);

        soniccore::Engine boosted(base_config);
        soniccore::Parameters eq;
        eq.eq_gain_db[5] = 6.0f;  // 1 kHz band.
        boosted.set_parameters(eq);
        auto boosted_signal = reference;
        boosted.process(boosted_signal.data(), boosted_signal.size() / 2);
        assert(rms(boosted_signal) > rms(reference) * 1.35f);

        soniccore::Engine dry_reverb(base_config);
        dry_reverb.set_effects_enabled(false, false, false, true, false);
        soniccore::Parameters no_wet;
        no_wet.reverb_mix = 0.0f;
        dry_reverb.set_parameters(no_wet);
        auto dry_signal = reference;
        dry_reverb.process(dry_signal.data(), dry_signal.size() / 2);
        for (std::size_t i = 0; i < dry_signal.size(); ++i) {
            assert(std::abs(dry_signal[i] - reference[i]) < 0.002f);
        }

        soniccore::Config spatial_config = base_config;
        spatial_config.spatial_enabled = true;
        soniccore::Engine spatial(spatial_config);
        soniccore::Parameters spatial_params;
        spatial_params.spatial_mix = 1.0f;
        spatial_params.spatial_width = 0.5f;
        spatial_params.spatial_rotate = true;
        spatial_params.spatial_pan_depth = 0.4f;
        spatial.set_parameters(spatial_params);
        auto spatial_signal = sine(sample_rate, 440.0f, 4096, 0.25f);
        spatial.process(spatial_signal.data(), spatial_signal.size() / 2);
        assert(peak(spatial_signal) <= 1.0f);
        assert(rms(spatial_signal) > 0.01f);
    }
    return 0;
}

