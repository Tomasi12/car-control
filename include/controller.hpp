#pragma once
#include <cstdint>

namespace ecu {

struct Config {
    std::int32_t max_speed_kmh;
    std::int32_t gain;
};

class Controller {
public:
    explicit Controller(const Config* cfg) : cfg_(cfg) {}
    std::int32_t torque_request(std::int32_t speed_kmh) const;
    std::int32_t mode_limit(int mode) const;

private:
    const Config* cfg_;
};

}  // namespace ecu
