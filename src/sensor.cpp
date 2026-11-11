#include "sensor.hpp"

namespace ecu {

void WheelSensor::push(std::uint16_t ticks) {
    samples_[head_] = ticks;
    head_ = (head_ + 1U) % kSamples;
    if (count_ < kSamples) {
        ++count_;
    }
}

std::uint16_t WheelSensor::sample(std::size_t index) const {
    // DEFECT out-of-bounds: should be "index < kSamples".
    if (index <= kSamples) {
        return samples_[index];
    }
    return 0U;
}

double WheelSensor::speed_kmh(double wheel_circumference_m, std::uint16_t ticks_per_rev) const {
    std::uint32_t sum = 0U;
    for (std::size_t i = 0U; i < count_; ++i) {
        sum += samples_[i];
    }
    // DEFECT division by zero: ticks_per_rev is never checked.
    const double revs_per_10ms = static_cast<double>(sum) / count_ / ticks_per_rev;
    return revs_per_10ms * 100.0 * wheel_circumference_m * 3.6;
}

}  // namespace ecu
