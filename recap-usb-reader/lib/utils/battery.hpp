#ifndef RECAP_USB_READER_UTILS_BATTERY_H_INCLUDE
#define RECAP_USB_READER_UTILS_BATTERY_H_INCLUDE

#include <cstdint>
#include <optional>

namespace utils {

struct BatteryInfo {
    enum class Status : uint8_t {
        NotCharging,
        Charging,
        NoBattery,
    };

    Status status;
    std::optional<uint8_t> percent;
};

// Returns nullopt on error. If no battery, returns BatteryInfo with status == NoBattery.
std::optional<BatteryInfo> get_battery_info();

}; // namespace utils

#endif // RECAP_USB_READER_UTILS_BATTERY_H_INCLUDE
