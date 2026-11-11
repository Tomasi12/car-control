#include "controller.hpp"

namespace ecu {

std::int32_t Controller::torque_request(std::int32_t speed_kmh) const {
    // DEFECT null dereference: cfg_ may be nullptr (see main.cpp).
    const std::int32_t error = cfg_->max_speed_kmh - speed_kmh;
    return error * cfg_->gain;
}

std::int32_t Controller::mode_limit(int mode) const {
    static const std::int32_t limits[3] = {100, 200, 300};
    // DEFECT out-of-bounds: mode is not range-checked.
    return limits[mode];
}

}  // namespace ecu
