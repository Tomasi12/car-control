#pragma once
#include <cstdint>
#include <vector>

namespace ecu {

std::int32_t average(const std::vector<std::int32_t>& values);
std::int32_t scale(std::int32_t value, std::int32_t numerator, std::int32_t denominator);
std::uint8_t to_percent(std::int32_t value, std::int32_t max_value);

}  // namespace ecu
