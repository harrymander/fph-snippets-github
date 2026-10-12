#include "battery.hpp"

#include <Windows.h>

#include <optional>

namespace utils {

std::optional<BatteryInfo> get_battery_info()
{
    SYSTEM_POWER_STATUS sys_status;
    if (!GetSystemPowerStatus(&sys_status)) {
        return std::nullopt;
    }

    if (sys_status.BatteryFlag == 255) {
        // Unknown status: unable to read the battery flag information
        return std::nullopt;
    }

    using enum BatteryInfo::Status;
    BatteryInfo info;
    if (sys_status.BatteryFlag & 128) {
        info.status = NoBattery;
        info.percent = std::nullopt;
    } else {
        info.status = (sys_status.BatteryFlag & 8) ? Charging : NotCharging;
        info.percent = sys_status.BatteryLifePercent <= 100 ?
            std::make_optional<uint8_t>(sys_status.BatteryLifePercent) :
            std::nullopt;
    }

    return info;
}

}; // namespace utils
