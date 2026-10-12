#include "directory.hpp"

#include <nfd.hpp>

using namespace recap::app::gui;
using namespace std::string_literals;

DirectorySelector::DirectorySelector(std::filesystem::path path) : m_path(std::move(path))
{
    if (NFD::Init() != NFD_OKAY) {
        throw std::runtime_error("NFD error: "s + NFD::GetError());
    }
}

DirectorySelector::DirectorySelector() : DirectorySelector(std::filesystem::current_path()) {}

DirectorySelector::~DirectorySelector() noexcept
{
    NFD::Quit();
}

bool DirectorySelector::select()
{
    // NFD should handle UTF-16 -> UTF-8 on Windows
    NFD::UniquePathU8 outpath;
    const auto result = NFD::PickFolder(outpath);
    if (result == NFD_OKAY) {
        m_path = std::filesystem::path(outpath.get());
    }
    return true;
}
