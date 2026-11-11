#include <cstdio>
#include <vector>

#include "controller.hpp"
#include "crc8.h"
#include "sensor.hpp"
#include "stats.hpp"

int main() {
    ecu::WheelSensor sensor;
    for (std::uint16_t t : {4, 5, 4, 5}) {
        sensor.push(t);
    }
    std::printf("speed      = %.1f km/h\n", sensor.speed_kmh(1.95, 48));
    std::printf("sample[2]  = %u\n", static_cast<unsigned>(sensor.sample(2)));

    const std::vector<std::int32_t> temps = {21, 23, 22, 24};
    std::printf("avg temp   = %d\n", ecu::average(temps));
    std::printf("scaled     = %d\n", ecu::scale(500, 3, 4));
    std::printf("percent    = %u\n", static_cast<unsigned>(ecu::to_percent(30, 120)));

    const ecu::Config cfg{130, 2};
    const ecu::Controller ctrl(&cfg);
    std::printf("torque     = %d\n", ctrl.torque_request(100));
    std::printf("mode limit = %d\n", ctrl.mode_limit(1));

    const std::uint8_t frame[] = {0x01, 0x02, 0x03, 0x00};
    std::printf("crc        = 0x%02X\n", static_cast<unsigned>(crc8(frame, 3)));

    // Paths that are only reached with bad input -- analyzers should still flag them:
    const bool fault_injection = false;
    if (fault_injection) {
        const ecu::Controller broken(nullptr);
        std::printf("%d\n", broken.torque_request(10));          // null dereference
        std::printf("%d\n", ecu::average({}));                   // division by zero
        std::printf("%u\n", static_cast<unsigned>(sensor.sample(8)));  // out-of-bounds
    }
    return 0;
}
