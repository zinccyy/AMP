#include <amp/utils/str.hpp>

namespace amp
{
namespace utils
{
namespace str
{
// replace all oldValue values with newValue
void replaceAllOccurrences(std::string &str, const std::string &oldValue, const std::string &newValue)
{
    std::size_t pos = 0;

    while ((pos = str.find(oldValue, pos)) != std::string::npos)
    {
        str.replace(pos, oldValue.size(), newValue);
        pos += newValue.size();
    }
}
} // namespace str
} // namespace utils
} // namespace amp
