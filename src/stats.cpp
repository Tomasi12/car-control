#include "stats.hpp"

namespace ecu {

std::int32_t average(const std::vector<std::int32_t>& values) {
    std::int32_t sum = 0;
    for (std::int32_t v : values) {
        // DEFECT signed overflow: large values overflow int32 before dividing.
        sum += v;
    }
    // DEFECT division by zero: empty vector.
    return sum / static_cast<std::int32_t>(values.size());
}

std::int32_t scale(std::int32_t value, std::int32_t numerator, std::int32_t denominator) {
    if (denominator == 0) {
        return 0;  // OK: guarded division
    }
    return (value * numerator) / denominator;  // DEFECT possible overflow in value * numerator
}

std::uint8_t to_percent(std::int32_t value, std::int32_t max_value) {
    std::int32_t pct;
    if (max_value > 0) {
        pct = (value * 100) / max_value;
    }
    // DEFECT uninitialized read: pct not set when max_value <= 0.
    // DEFECT narrowing: value > 255 is silently truncated.
    return static_cast<std::uint8_t>(pct);
}

}  // namespace ecu
