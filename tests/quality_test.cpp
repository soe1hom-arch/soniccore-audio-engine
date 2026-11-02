#include "soniccore/sonic_core.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace {
float peak(const std::vector<float>& samples) {
    float result = 0.0f;
    for (float sample : samples) result = std::max(result, std::abs(sample));
    return result;
}
}

int main() {
    soniccore::Config config;
    config.headroom_db = 0.0f;
    config.limiter_enabled = false;
    soniccore::Engine engine(config);

    // Flat settings are effectively transparent.
    std::vector<float> input(512);
    for (std::size_t i = 0; i < input.size(); ++i) {
        input[i] = 0.25f * std::sin(static_cast<float>(i) * 0.17f);
    }
    const auto original = input;
    engine.process(input.data(), input.size() / 2);
    float error = 0.0f;
    for (std::size_t i = 0; i < input.size(); ++i) error = std::max(error, std::abs(input[i] - original[i]));
    assert(error < 0.001f);

    // The final safety stage must never emit above full scale.
    config.limiter_enabled = true;
    config.limiter_ceiling = 0.98f;
    soniccore::Engine limited(config);
    std::vector<float> hot(512, 1.8f);
    limited.process(hot.data(), hot.size() / 2);
    assert(peak(hot) <= 1.0f);

    // Bypass must preserve samples exactly, including out-of-range test data.
    limited.set_bypassed(true);
    std::vector<float> bypassed = {1.2f, -1.2f, 0.3f, -0.3f};
    const auto before_bypass = bypassed;
    limited.process(bypassed.data(), bypassed.size() / 2);
    assert(bypassed == before_bypass);
    return 0;
}
