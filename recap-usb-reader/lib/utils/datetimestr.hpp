#ifndef RECAP_USB_READER_UTILS_DATETIMESTR_H_INCLUDE
#define RECAP_USB_READER_UTILS_DATETIMESTR_H_INCLUDE

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace utils {

inline std::string utc_datetime_str()
{
    using clock_type = std::chrono::system_clock;
    const std::time_t time = clock_type::to_time_t(clock_type::now());

    std::stringstream os;
    os << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S+0000");
    return os.str();
}

}; // namespace utils

#endif // RECAP_USB_READER_UTILS_DATETIMESTR_H_INCLUDE
