#pragma once

#include <string_view>

namespace recap::app::filename {

static constexpr unsigned int MAX_FILENAME_LEN = 255;
static constexpr const char *ALLOWED_FILENAME_SYMBOLS = ")(][+,.-_";

bool is_valid_filename(std::string_view name);

}; // namespace recap::app::filename
