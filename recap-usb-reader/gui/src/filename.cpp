#include "filename.hpp"

#include <algorithm>

bool recap::app::filename::is_valid_filename(std::string_view name)
{
    if (name.length() > MAX_FILENAME_LEN) {
        return false;
    }

    static constexpr std::string_view allowed_view(ALLOWED_FILENAME_SYMBOLS);
    return std::all_of(name.begin(), name.end(), [](const auto& c) {
        return (
            (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
            || std::any_of(
                allowed_view.begin(), allowed_view.end(), [&c](const auto& sym) { return c == sym; }
            )
        );
    });
}
