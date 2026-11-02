#include "soniccore/sonic_core.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace soniccore {
namespace {
constexpr float kPi = 3.14159265358979323846f;

float db_to_gain(float db) noexcept {
    return std::pow(10.0f, db / 20.0f);
}

std::array<float, 5> peaking(float fs, float fc, float gain_db) noexcept {
    const float a = std::pow(10.0f, gain_db / 40.0f);
    const float w0 = 2.0f * kPi * fc / fs;
    const float c = std::cos(w0);
    const float alpha = std::sin(w0) / (2.0f * 0.71f);
    const float b0 = 1.0f + alpha * a;
    const float b1 = -2.0f * c;
    const float b2 = 1.0f - alpha * a;
    const float a0 = 1.0f + alpha / a;
    const float a1 = -2.0f * c;
    const float a2 = 1.0f - alpha / a;
    const float n = 1.0f / a0;
    return {b0 * n, b1 * n, b2 * n, a1 * n, a2 * n};
}

std::array<float, 5> shelf(float fs, float fc, float gain_db, bool low) noexcept {
    const float a = std::pow(10.0f, gain_db / 40.0f);
    const float w0 = 2.0f * kPi * fc / fs;
    const float c = std::cos(w0);
    const float alpha = std::sin(w0) / (2.0f * 0.71f);
    const float sqrt_a = std::sqrt(a);
    const float sign_c = low ? -1.0f : 1.0f;
    const float sign_a1 = low ? 1.0f : -1.0f;
    const float ap1 = a + 1.0f;
    const float am1 = a - 1.0f;
    const float b0 = a * (ap1 + am1 * c * sign_c + 2.0f * sqrt_a * alpha);
    const float b1 = -2.0f * a * ((a + 1.0f) * c + am1 * sign_c);
    const float b2 = a * (ap1 + am1 * c * sign_c - 2.0f * sqrt_a * alpha);
    const float a0 = ap1 - am1 * c * sign_c + 2.0f * sqrt_a * alpha;
    const float a1 = -2.0f * (am1 * sign_a1 + (a + 1.0f) * c);
    const float a2 = ap1 - am1 * c * sign_c - 2.0f * sqrt_a * alpha;
    const float n = 1.0f / a0;
    return {b0 * n, b1 * n, b2 * n, a1 * n, a2 * n};
}
}

float Engine::Biquad::process(float input) noexcept {
    const float output = c[0] * input + c[1] * x1 + c[2] * x2 - c[3] * y1 - c[4] * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return output;
}

void Engine::Biquad::reset() noexcept {
    x1 = x2 = y1 = y2 = 0.0f;
}

Engine::Engine(const Config& config) : config_(config) {
    config_.channels = std::max(1, config_.channels);
    config_.sample_rate = std::max(1, config_.sample_rate);
    config_.limiter_ceiling = std::clamp(config_.limiter_ceiling, 0.1f, 1.0f);
    channel_eq_.resize(static_cast<std::size_t>(config_.channels * 10));
    channel_tone_.resize(static_cast<std::size_t>(config_.channels * 2));
    reverb_.resize(static_cast<std::size_t>(config_.channels));
    spatial_delay_left_.resize(static_cast<std::size_t>(config_.sample_rate * 0.38f));
    spatial_delay_right_.resize(spatial_delay_left_.size());
    reset_reverb();
    update_coefficients();
}

void Engine::set_parameters(const Parameters& parameters) {
    parameters_ = parameters;
    update_coefficients();
}

void Engine::set_effects_enabled(bool eq, bool tone, bool spatial, bool reverb,
                                 bool limiter) noexcept {
    config_.eq_enabled = eq;
    config_.tone_enabled = tone;
    config_.spatial_enabled = spatial;
    config_.reverb_enabled = reverb;
    config_.limiter_enabled = limiter;
}

void Engine::set_bypassed(bool bypassed) {
    bypassed_ = bypassed;
}

bool Engine::bypassed() const noexcept {
    return bypassed_;
}

void Engine::reset() noexcept {
    envelope_ = 0.0f;
    phase_ = 0.0f;
    for (auto& filter : channel_eq_) filter.reset();
    for (auto& filter : channel_tone_) filter.reset();
    reset_reverb();
    std::fill(spatial_delay_left_.begin(), spatial_delay_left_.end(), 0.0f);
    std::fill(spatial_delay_right_.begin(), spatial_delay_right_.end(), 0.0f);
    spatial_delay_position_ = 0;
}

void Engine::process(float* interleaved, std::size_t frames) noexcept {
    if (interleaved == nullptr || frames == 0 || bypassed_) return;

    const float trim = db_to_gain(config_.input_trim_db + config_.output_trim_db +
                                  config_.headroom_db);
    const float ceiling = config_.limiter_ceiling;
    const float spatial_depth = std::clamp(parameters_.spatial_mix, 0.0f, 1.0f);
    const float reverb_mix = std::clamp(parameters_.reverb_mix, 0.0f, 1.0f);

    for (std::size_t frame = 0; frame < frames; ++frame) {
        float left = interleaved[frame * config_.channels] * trim;
        float right = config_.channels > 1 ? interleaved[frame * config_.channels + 1] * trim : left;

        for (int channel = 0; channel < config_.channels; ++channel) {
            float sample = interleaved[frame * config_.channels + channel] * trim;
            if (config_.eq_enabled) {
                for (int band = 0; band < 10; ++band) {
                    sample = channel_eq_[static_cast<std::size_t>(channel * 10 + band)].process(sample);
                }
            }
            if (config_.tone_enabled) {
                sample = channel_tone_[static_cast<std::size_t>(channel * 2)].process(sample);
                sample = channel_tone_[static_cast<std::size_t>(channel * 2 + 1)].process(sample);
            }
            interleaved[frame * config_.channels + channel] = sample;
        }
        left = interleaved[frame * config_.channels];
        right = config_.channels > 1 ? interleaved[frame * config_.channels + 1] : left;

        // The initial core keeps the contract and safety stages explicit. EQ,
        // spatial, and reverb implementations are added behind this stable API.
        if (config_.spatial_enabled && spatial_depth > 0.0f && config_.channels > 1) {
            const float mid = (left + right) * 0.5f;
            const float side = (left - right) * 0.5f;
            const float width = 1.0f + std::clamp(parameters_.spatial_width, 0.0f, 1.0f) * 1.8f;
            const float mid_gain = 1.0f + (width - 1.0f) * 0.35f;
            left = mid * mid_gain + side * width;
            right = mid * mid_gain - side * width;

            if (parameters_.spatial_rotate) {
                const float pan = std::sin(phase_) *
                    std::clamp(parameters_.spatial_pan_depth, 0.1f, 1.0f);
                const float theta = (pan + 1.0f) * (kPi * 0.25f);
                const float gain_l = std::cos(theta);
                const float gain_r = std::sin(theta);
                left *= gain_l * 1.41421356f;
                right *= gain_r * 1.41421356f;
                phase_ += 2.0f * kPi * std::max(0.01f, parameters_.spatial_rate_hz) /
                          static_cast<float>(config_.sample_rate);
                if (phase_ > 2.0f * kPi) phase_ -= 2.0f * kPi;
            }
        }

        if (config_.reverb_enabled && reverb_mix > 0.0f && config_.channels > 1) {
            const std::size_t delay_size = spatial_delay_left_.size();
            const std::size_t read = (spatial_delay_position_ + 1) % delay_size;
            const float echo_l = spatial_delay_left_[read];
            const float echo_r = spatial_delay_right_[read];
            const float feedback = 0.60f + std::clamp(parameters_.reverb_room, 0.0f, 1.0f) * 0.22f;
            const float wet = std::clamp(reverb_mix, 0.0f, 1.0f);
            const float room_l = schroeder(reverb_[0], left, feedback);
            const float room_r = schroeder(reverb_[1], right, feedback);
            left = left * (1.0f - wet) + (room_l * 0.3f + echo_l * 0.1f) * wet;
            right = right * (1.0f - wet) + (room_r * 0.3f + echo_r * 0.1f) * wet;
            spatial_delay_left_[spatial_delay_position_] = left;
            spatial_delay_right_[spatial_delay_position_] = right;
            spatial_delay_position_ = (spatial_delay_position_ + 1) % delay_size;
        }

        if (config_.limiter_enabled) {
            const float peak = std::max(std::abs(left), std::abs(right));
            envelope_ = std::max(peak, envelope_ * 0.995f);
            const float gain = envelope_ > ceiling ? ceiling / envelope_ : 1.0f;
            left *= gain;
            right *= gain;
        }

        interleaved[frame * config_.channels] = std::clamp(left, -1.0f, 1.0f);
        if (config_.channels > 1) {
            interleaved[frame * config_.channels + 1] = std::clamp(right, -1.0f, 1.0f);
        }
        for (int channel = 2; channel < config_.channels; ++channel) {
            interleaved[frame * config_.channels + channel] =
                std::clamp(interleaved[frame * config_.channels + channel], -1.0f, 1.0f);
        }
    }
}

float Engine::schroeder_sample(DelayLine& line, float input, float feedback) noexcept {
    const float delayed = line.buffer[line.position];
    line.damped += (delayed - line.damped) * 0.65f;
    line.buffer[line.position] = input + line.damped * feedback;
    line.position = (line.position + 1) % line.buffer.size();
    return delayed;
}

float Engine::schroeder(ReverbChannel& channel, float input, float feedback) noexcept {
    float wet = 0.0f;
    for (auto& comb : channel.combs) wet += schroeder_sample(comb, input, feedback);
    for (auto& allpass : channel.allpasses) {
        const float buffered = allpass.buffer[allpass.position];
        const float output = -wet + buffered;
        allpass.buffer[allpass.position] = wet + buffered * 0.5f;
        allpass.position = (allpass.position + 1) % allpass.buffer.size();
        wet = output;
    }
    return wet;
}

void Engine::reset_reverb() noexcept {
    static constexpr float comb_seconds[4] = {0.0297f, 0.0371f, 0.0411f, 0.0437f};
    static constexpr float allpass_seconds[2] = {0.0051f, 0.0017f};
    for (std::size_t channel = 0; channel < reverb_.size(); ++channel) {
        for (std::size_t i = 0; i < reverb_[channel].combs.size(); ++i) {
            const auto samples = static_cast<std::size_t>(comb_seconds[i] * config_.sample_rate) +
                (channel % 2 == 1 ? 23u : 0u);
            reverb_[channel].combs[i].buffer.assign(std::max<std::size_t>(1, samples), 0.0f);
            reverb_[channel].combs[i].position = 0;
            reverb_[channel].combs[i].damped = 0.0f;
        }
        for (std::size_t i = 0; i < reverb_[channel].allpasses.size(); ++i) {
            const auto samples = static_cast<std::size_t>(allpass_seconds[i] * config_.sample_rate);
            reverb_[channel].allpasses[i].buffer.assign(std::max<std::size_t>(1, samples), 0.0f);
            reverb_[channel].allpasses[i].position = 0;
        }
    }
}

void Engine::update_coefficients() noexcept {
    static constexpr float frequencies[10] = {
        31.25f, 62.5f, 125.0f, 250.0f, 500.0f,
        1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
    };
    for (int band = 0; band < 10; ++band) {
        eq_[band].c = peaking(static_cast<float>(config_.sample_rate), frequencies[band],
                               std::clamp(parameters_.eq_gain_db[band], -15.0f, 15.0f));
    }
    tone_[0].c = shelf(static_cast<float>(config_.sample_rate), 150.0f,
                       std::clamp(parameters_.bass_db, -6.0f, 6.0f), true);
    tone_[1].c = shelf(static_cast<float>(config_.sample_rate), 3200.0f,
                       std::clamp(parameters_.treble_db, -6.0f, 6.0f), false);
    for (int channel = 0; channel < config_.channels; ++channel) {
        for (int band = 0; band < 10; ++band) {
            channel_eq_[static_cast<std::size_t>(channel * 10 + band)].c = eq_[band].c;
        }
        for (int band = 0; band < 2; ++band) {
            channel_tone_[static_cast<std::size_t>(channel * 2 + band)].c = tone_[band].c;
        }
    }
}

}  // namespace soniccore
