#pragma once

#include <string>

namespace amp
{
namespace utils
{
namespace str
{
// replace all oldValue values with newValue
void replaceAllOccurrences(std::string &str, const std::string &oldValue, const std::string &newValue);
} // namespace str
} // namespace utils
} // namespace amp
