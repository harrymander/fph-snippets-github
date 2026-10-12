#include "record.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace recap::record {

static std::string::size_type next_sep(std::string_view str, std::string::size_type pos)
{
    const auto ret = str.find('.', pos);
    if (ret == std::string::npos) {
        throw std::invalid_argument("missing dot separator");
    }
    return ret;
}

static unsigned int parse_num(std::string_view s)
{
    unsigned int num;
    const char *const last = s.data() + s.size();
    const auto [ptr, ec] = std::from_chars(s.data(), last, num, 10);
    if (ec != std::errc() || ptr != last) {
        throw std::invalid_argument("invalid number");
    }
    return num;
}

std::string FirmwareVersion::str() const
{
    std::stringstream buf;
    buf << major << '.' << minor << '.' << patch;
    if (extra.has_value()) {
        buf << '-' << *extra;
    }
    return buf.str();
}

static void check_extra(const std::string& str)
{
    if (str.empty()) {
        throw std::invalid_argument("extra version info cannot be empty string");
    }
    if (std::any_of(str.begin(), str.end(), [](const auto& c) { return std::isspace(c); })) {
        throw std::invalid_argument("extra version info cannot contain whitespace");
    }
}

FirmwareVersion::FirmwareVersion(
    unsigned int major, unsigned int minor, unsigned int patch, std::string_view extra
) :
    major(major), minor(minor), patch(patch), extra(extra)
{
    check_extra(*this->extra);
}

FirmwareVersion::FirmwareVersion(std::string_view string)
{
    if (string.empty()) {
        throw std::invalid_argument("empty string");
    }

    auto pos = next_sep(string, 0);
    major = parse_num(string.substr(0, pos));
    pos += 1;

    auto next = next_sep(string, pos);
    minor = parse_num(string.substr(pos, next - pos));
    pos = next + 1;

    next = string.find('-', pos);
    if (next == std::string::npos) {
        patch = parse_num(string.substr(pos));
    } else {
        patch = parse_num(string.substr(pos, next - pos));
        const auto extra_str = string.substr(next + 1);
        if (extra_str.empty()) {
            throw std::invalid_argument("empty extra version info string");
        }
        extra = std::make_optional(extra_str);
        check_extra(*extra);
    }
}

}; // namespace recap::record
