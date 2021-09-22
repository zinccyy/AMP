#pragma once

#include <string>

namespace amp
{
namespace db
{
namespace scheme
{
struct Track
{
    // track_id
    int ID;

    // track_name
    std::string Name;

    // track_file_path
    std::string FilePath;

    // track_length
    int LengthInSeconds;

    // track_number
    unsigned int Number;

    // track_comment
    std::string Comment;

    // track_bit_rate
    int Bitrate;

    // track_sample_rate
    int SampleRate;

    // track_channels
    int Channels;

    // album_id
    int AlbumID;
};
} // namespace scheme
} // namespace db
} // namespace amp
