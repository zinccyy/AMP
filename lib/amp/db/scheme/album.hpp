#pragma once

#include <string>

namespace amp
{
namespace db
{
namespace scheme
{
struct Album
{
    // album_id
    int ID;

    // album_name
    std::string Name;

    // album_folder_path
    std::string FolderPath;

    // album_cover
    std::string CoverPath;

    // album_year
    unsigned int Year;

    // artist_id
    int ArtistID;

    // genre_id
    int GenreID;
};
} // namespace scheme
} // namespace db
} // namespace amp
