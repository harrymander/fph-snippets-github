// Avoids warning about getenv on MSVC
#define _CRT_SECURE_NO_WARNINGS 1

#include "font.hpp"

#include <config/font.h>

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

static ImFont *g_font_handle;

extern "C" const uint8_t fontawesome_ttf_data[];
extern "C" const size_t fontawesome_ttf_data_size;

static int detect_font_size(float& val)
{
    const char *size_env = std::getenv("RECAP_USB_READER_GUI_FONT_SIZE");
    if (size_env == nullptr) {
        return -1;
    }

    const auto result = std::from_chars(size_env, size_env + std::strlen(size_env), val);
    if (result.ec != std::errc() || val <= 0) {
        return -1;
    }
    return 0;
}

#ifdef RECAP_USB_READER_GUI_EMBED_FONT
extern "C" const uint8_t recap_font_data[];
extern "C" const size_t recap_font_data_size;

static int add_main_font(float scaling_factor)
{
    float size;
    if (detect_font_size(size) < 0) {
        size = RECAP_USB_READER_GUI_FONT_SIZE;
    }
    if (scaling_factor > 0) {
        size *= scaling_factor;
    }

    auto *atlas = ImGui::GetIO().Fonts;
    static ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    if (atlas->AddFontFromMemoryTTF(
            const_cast<void *>(static_cast<const void *>(recap_font_data)),
            recap_font_data_size,
            size,
            &cfg,
            atlas->GetGlyphRangesDefault()
        )
        == nullptr)
    {
        return -1;
    }

    return 0;
}
#else
constexpr float BUILD_FONT_SIZE = RECAP_USB_READER_GUI_FONT_SIZE;
constexpr const char *BUILD_FONT_PATH = RECAP_USB_READER_GUI_FONT_PATH;

static int detect_font_path(std::filesystem::path& path)
{
    const char *path_env = std::getenv("RECAP_USB_READER_GUI_FONT_PATH");
    if (path_env == nullptr) {
        return -1;
    }

    path = std::filesystem::path(path_env);
    if (!std::filesystem::exists(path)) {
        return -1;
    }

    return 0;
}

static int add_default_font()
{
    auto *font_atlas = ImGui::GetIO().Fonts;
    static ImFontConfig config;
    config.GlyphRanges = font_atlas->GetGlyphRangesDefault();
    return font_atlas->AddFontDefault(&config) == nullptr ? -1 : 0;
}

static int detect_font_settings(float& size, std::filesystem::path& path)
{
    if (detect_font_size(size) < 0) {
        if constexpr (BUILD_FONT_SIZE <= 0) {
            return -1;
        }
        size = BUILD_FONT_SIZE;
    }

    if (detect_font_path(path) < 0) {
        if (BUILD_FONT_PATH == nullptr) {
            return -1;
        }
        path = std::filesystem::path(BUILD_FONT_PATH);
    }

    return 0;
}

static int add_main_font(float scaling_factor)
{
    float size;
    std::filesystem::path path;

    // Try to get size and path from environment, then fall back to compile constants, then fall
    // back to builtin DearImgui font
    if (detect_font_settings(size, path) < 0) {
        return add_default_font();
    }

    if (scaling_factor > 0) {
        size *= scaling_factor;
    }

    auto *atlas = ImGui::GetIO().Fonts;
    if (atlas->AddFontFromFileTTF(
            path.string().c_str(), size, nullptr, atlas->GetGlyphRangesDefault()
        )
        == nullptr)
    {
        return -1;
    }

    return 0;
}
#endif

static int merge_icon_font()
{
    // Merge the icon font into main font
    static ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.MergeMode = true;
    static const ImWchar glyph_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    auto& font_atlas = ImGui::GetIO().Fonts;
    g_font_handle = font_atlas->AddFontFromMemoryTTF(
        const_cast<void *>(static_cast<const void *>(fontawesome_ttf_data)),
        fontawesome_ttf_data_size,
        font_atlas->ConfigData.back().SizePixels,
        &font_cfg,
        glyph_ranges
    );
    return g_font_handle == nullptr ? -1 : 0;
}

int font::init_fonts(float scaling_factor)
{
    if (add_main_font(scaling_factor) < 0) {
        return -1;
    }

    return merge_icon_font();
}
