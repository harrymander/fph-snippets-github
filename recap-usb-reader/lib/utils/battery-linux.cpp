#include "battery.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#define BAT_SYSFS "/sys/class/power_supply/BAT0/"

namespace utils {

namespace {

std::optional<uint8_t> read_capacity()
{
    int capacity;
    std::ifstream f(BAT_SYSFS "capacity");
    if (f && (f >> capacity) && capacity >= 0 && capacity <= 100) {
        return capacity;
    }

    return std::nullopt;
}

std::optional<BatteryInfo::Status> read_status()
{
    using enum BatteryInfo::Status;

    std::string status;
    std::ifstream f(BAT_SYSFS "status");
    if (f && (f >> status)) {
        std::ranges::transform(status, status.begin(), [](auto c) { return std::tolower(c); });
        return status == "charging" ? Charging : NotCharging;
    }
    return std::nullopt;
}

}; // namespace

std::optional<BatteryInfo> get_battery_info()
{
    BatteryInfo info;
    if (std::filesystem::exists(BAT_SYSFS)) {
        auto status = read_status();
        if (!status) {
            return std::nullopt;
        }
        info.status = *status;
        info.percent = read_capacity();
    } else {
        info.status = BatteryInfo::Status::NoBattery;
        info.percent = std::nullopt;
    }

    return info;
}

}; // namespace utils
