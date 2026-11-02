#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>

namespace soniccore {

struct Config {
    int sample_rate = 48000;
    int channels = 2;
    float input_trim_db = 0.0f;
    float output_trim_db = 0.0f;
    float headroom_db = -1.5f;
    float limiter_ceiling = 0.98f;
    bool eq_enabled = true;
    bool tone_enabled = true;
    bool spatial_enabled = false;
    bool reverb_enabled = false;
    bool limiter_enabled = true;
};

struct Parameters {
    float eq_gain_db[10]{};
    float bass_db = 0.0f;
    float treble_db = 0.0f;
    float spatial_mix = 0.0f;
    float spatial_rate_hz = 0.15f;
    float spatial_width = 0.35f;
    float spatial_pan_depth = 0.4f;
    bool spatial_rotate = false;
    bool spatial_surround = false;
    float reverb_mix = 0.0f;
    float reverb_room = 0.5f;
};

class Engine final {
public:
    explicit Engine(const Config& config);

    void set_parameters(const Parameters& parameters);
    void set_effects_enabled(bool eq, bool tone, bool spatial, bool reverb, bool limiter) noexcept;
    void set_bypassed(bool bypassed);
    bool bypassed() const noexcept;

    // Interleaved float PCM. The caller owns the buffer and frame count.
    // The real-time contract: no allocation, I/O, or locks in this method.
    void process(float* interleaved, std::size_t frames) noexcept;

    void reset() noexcept;

private:
    struct Biquad {
        std::array<float, 5> c{};
        float x1 = 0.0f;
        float x2 = 0.0f;
        float y1 = 0.0f;
        float y2 = 0.0f;

        float process(float input) noexcept;
        void reset() noexcept;
    };

    struct DelayLine {
        std::vector<float> buffer;
        std::size_t position = 0;
        float damped = 0.0f;
    };

    struct ReverbChannel {
        std::array<DelayLine, 4> combs;
        std::array<DelayLine, 2> allpasses;
    };

    Config config_;
    Parameters parameters_;
    std::array<Biquad, 10> eq_{};
    std::array<Biquad, 2> tone_{};
    std::vector<Biquad> channel_eq_;
    std::vector<Biquad> channel_tone_;
    std::vector<ReverbChannel> reverb_;
    bool bypassed_ = false;
    float envelope_ = 0.0f;
    float phase_ = 0.0f;
    std::size_t spatial_delay_position_ = 0;
    std::vector<float> spatial_delay_left_;
    std::vector<float> spatial_delay_right_;

    void update_coefficients() noexcept;
    void reset_reverb() noexcept;
    static float schroeder_sample(DelayLine& line, float input, float feedback) noexcept;
    static float schroeder(ReverbChannel& channel, float input, float feedback) noexcept;
};

}  // namespace soniccore
