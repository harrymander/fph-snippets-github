#include "strhelpers.hpp"

#include <imgui.h>

#include <algorithm>
#include <string>

namespace strhelpers {

template <class UnaryPredicate> static void lstrip(std::string& s, UnaryPredicate q)
{
    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), q));
}

void lstrip(std::string& s)
{
    lstrip(s, [](auto c) { return std::isspace(c); });
}

template <class UnaryPredicate> void rstrip(std::string& s, UnaryPredicate q)
{
    s.erase(std::find_if_not(s.rbegin(), s.rend(), q).base(), s.end());
}

void rstrip(std::string& s)
{
    rstrip(s, [](auto c) { return std::isspace(c); });
}

void strip(std::string& s)
{
    lstrip(s);
    rstrip(s);
}

}; // namespace strhelpers
