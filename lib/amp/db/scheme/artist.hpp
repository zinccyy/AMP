#pragma once

#include <string>

namespace amp
{
namespace db
{
namespace scheme
{
struct Artist
{
    // artist_id
    int ID;

    // artist_name
    std::string Name;

    // artist_image
    std::string ImagePath;
};
} // namespace scheme
} // namespace db
} // namespace amp
