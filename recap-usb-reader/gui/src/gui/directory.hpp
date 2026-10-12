#pragma once

#include <filesystem>

namespace recap::app::gui {

class DirectorySelector {
public:
    explicit DirectorySelector(std::filesystem::path path);

    DirectorySelector();

    ~DirectorySelector() noexcept;

    DirectorySelector(const DirectorySelector&) = delete;
    DirectorySelector& operator=(const DirectorySelector&) = delete;
    DirectorySelector(DirectorySelector&&) = delete;
    DirectorySelector& operator=(DirectorySelector&&) = delete;

    const std::filesystem::path& path() const { return m_path; }

    std::filesystem::path file_path(std::string_view fname) const
    {
        return path() / std::filesystem::path(fname);
    }

    bool exists() const { return std::filesystem::exists(m_path); }

    bool select();

private:
    std::filesystem::path m_path;
};

}; // namespace recap::app::gui
