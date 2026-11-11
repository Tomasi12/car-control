#pragma once
#include <array>
#include <cstdint>

namespace ecu {

// Wheel-speed sensor: stores the last N raw samples (ticks per 10 ms).
class WheelSensor {
public:
    static constexpr std::size_t kSamples = 8;

    void push(std::uint16_t ticks);
    std::uint16_t sample(std::size_t index) const;
    std::size_t count() const { return count_; }
    double speed_kmh(double wheel_circumference_m, std::uint16_t ticks_per_rev) const;

private:
    std::array<std::uint16_t, kSamples> samples_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
};

}  // namespace ecu
