#include "soniccore/profiles.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    constexpr int sample_rate = 48000;
    constexpr int frames_per_buffer = 256;
    constexpr int iterations = 10000;
    auto settings = soniccore::make_profile(soniccore::Profile::Room, sample_rate, 2);
    soniccore::Engine engine(settings.config);
    engine.set_parameters(settings.parameters);
    std::vector<float> buffer(frames_per_buffer * 2, 0.1f);

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        engine.process(buffer.data(), frames_per_buffer);
        buffer[0] = std::sin(static_cast<float>(i) * 0.01f) * 0.1f;
    }
    const auto elapsed = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start).count();
    const double audio_ms = iterations * frames_per_buffer * 1000.0 / sample_rate;
    std::cout << "processed " << audio_ms << " ms audio in " << elapsed
              << " ms (" << (elapsed / audio_ms * 100.0) << "% realtime)\n";
    return 0;
}

