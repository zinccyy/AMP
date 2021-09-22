#include <amp/db/database_manager.hpp>
#include <amp/log.hpp>

#include <memory>

namespace amp
{
namespace db
{
DatabaseManager::DatabaseManager() : mConn(nullptr), mTx(nullptr), mArtistID(0), mAlbumID(0), mGenreID(0), mTrackID(0)
{
}
// initializes the connection with the database
void DatabaseManager::initConnection(const char *dbName, const char *username, const char *password)
{
    char options_buffer[2048];

    snprintf(options_buffer, sizeof(options_buffer), "host=localhost port=5432 dbname=%s connect_timeout=2 password=%s user=%s", dbName, username, password);

    mConn = std::make_unique<pqxx::connection>(options_buffer);
}
// loads the current database table data for quicker access later on
int DatabaseManager::loadCache()
{
    int error = 0;
    try
    {
        startTransaction();

        auto id_transform = [](const pqxx::row &row) -> int { return row[0].is_null() ? 0 : row[0].as<int>(); };

        auto artist_transform = [](const pqxx::row &row) {
            return db::scheme::Artist{
                .ID = row[0].as<int>(),
                .Name = row[1].as<std::string>(),
            };
        };

        auto album_transform = [](const pqxx::row &row) {
            return db::scheme::Album{
                .ID = row[0].as<int>(),
                .Name = row[1].as<std::string>(),
            };
        };

        auto genre_transform = [](const pqxx::row &row) {
            return db::scheme::Genre{
                .ID = row[0].as<int>(),
                .Name = row[1].as<std::string>(),
            };
        };

        // get the last IDs
        mArtistID = getRow<int>("SELECT MAX(artist_id) FROM artist;", id_transform) + 1;
        mAlbumID = getRow<int>("SELECT MAX(album_id) FROM album;", id_transform) + 1;
        mGenreID = getRow<int>("SELECT MAX(genre_id) FROM genre;", id_transform) + 1;
        mTrackID = getRow<int>("SELECT MAX(track_id) FROM track;", id_transform) + 1;

        // get all artists and at the init stage create cache for faster usage later
        auto artists = getRows<db::scheme::Artist>("SELECT artist_id, artist_name FROM artist", artist_transform);
        auto albums = getRows<db::scheme::Album>("SELECT album_id, album_name FROM album", album_transform);
        auto genres = getRows<db::scheme::Genre>("SELECT genre_id, genre_name FROM genre", genre_transform);

        commitTransaction();

        for (auto artist : artists)
        {
            mArtistCache[artist.Name] = artist.ID;
        }

        for (auto album : albums)
        {
            mAlbumCache[album.Name] = album.ID;
        }

        for (auto genre : genres)
        {
            mGenreCache[genre.Name] = genre.ID;
        }
    }
    catch (const std::exception &e)
    {
        AMP_LOG_ERR("database error: %s", e.what());
        AMP_LOG_ERR("unable to load cache");
        error = -1;
    }

    return error;
}
// starts a new transaction
void DatabaseManager::startTransaction()
{
    mTx = std::make_unique<pqxx::work>(*mConn);
}

// commits the current transaction
void DatabaseManager::commitTransaction()
{
    mTx->commit();

    // free the transaction pointer - not longer needed until next transaction
    (void)mTx.release();
}
// some specific functions for inserting cxx structs into the database
int DatabaseManager::insertArtist(const scheme::Artist &artist)
{
    int error = 0;
    char query_buffer[2048] = {0};

    snprintf(query_buffer, sizeof(query_buffer), "INSERT INTO artist(artist_id, artist_name) VALUES(%d, \'%s\');", mArtistID, mTx->esc(artist.Name).c_str());
    AMP_LOG_INF("running query: \"%s\"", query_buffer);

    auto insert_res = mTx->exec(query_buffer);
    if (insert_res.affected_rows() == 0)
    {
        error = -1;
    }
    else
    {
        mArtistCache[artist.Name] = mArtistID;
        ++mArtistID;
    }

    return error;
}
int DatabaseManager::insertGenre(const scheme::Genre &genre)
{
    int error = 0;
    char query_buffer[2048] = {0};

    snprintf(query_buffer, sizeof(query_buffer), "INSERT INTO genre(genre_id, genre_name) VALUES(%d, \'%s\');", mGenreID, mTx->esc(genre.Name).c_str());
    AMP_LOG_INF("running query: \"%s\"", query_buffer);

    auto insert_res = mTx->exec(query_buffer);
    if (insert_res.affected_rows() == 0)
    {
        error = -1;
    }
    else
    {
        mGenreCache[genre.Name] = mGenreID;
        ++mGenreID;
    }

    return error;
}
int DatabaseManager::insertAlbum(const scheme::Album &album, const std::string &artist, const std::string &genre)
{
    int error = 0;
    char query_buffer[2048] = {0};

    snprintf(query_buffer, sizeof(query_buffer),
             "INSERT INTO album(album_id, album_name, album_cover, album_year, album_folder_path, artist_id, genre_id) VALUES(%d, \'%s\', \'%s\', \'%d-01-01\', "
             "\'%s\', %d, %d);",
             mAlbumID, mTx->esc(album.Name).c_str(), mTx->esc(album.CoverPath).c_str(), album.Year, mTx->esc(album.FolderPath).c_str(), mArtistCache[artist], mGenreCache[genre]);
    AMP_LOG_INF("running query: \"%s\"", query_buffer);

    auto insert_res = mTx->exec(query_buffer);
    if (insert_res.affected_rows() == 0)
    {
        error = -1;
    }
    else
    {
        mAlbumCache[album.Name] = mAlbumID;
        ++mAlbumID;
    }
    return error;
}
int DatabaseManager::insertTrack(const scheme::Track &track, const std::string &album)
{
    int error = 0;
    char query_buffer[2048] = {0};

    // insert track
    snprintf(query_buffer, sizeof(query_buffer),
             "INSERT INTO track(track_id, track_title, track_length, track_number, track_comment, track_bit_rate, track_sample_rate, track_channels, track_file_path, "
             "album_id) VALUES(%d, "
             "\'%s\', \'%d seconds\'::interval::time(0), %d, \'%s\', %d, %d, %d, \'%s\', %d);",
             mTrackID, mTx->esc(track.Name).c_str(), track.LengthInSeconds, track.Number, mTx->esc(track.Comment).c_str(), track.Bitrate, track.SampleRate, track.Channels,
             mTx->esc(track.FilePath).c_str(), mAlbumCache[album]);
    AMP_LOG_INF("running query: \"%s\"", query_buffer);

    auto insert_res = mTx->exec(query_buffer);
    if (insert_res.affected_rows() == 0)
    {
        error = -1;
    }
    else
    {
        ++mTrackID;
    }

    return error;
}
// check wether the value exists in the database
bool DatabaseManager::artistExists(const std::string &name)
{
    return mArtistCache.find(name) != mArtistCache.end();
}
bool DatabaseManager::genreExists(const std::string &name)
{
    return mGenreCache.find(name) != mGenreCache.end();
}
bool DatabaseManager::albumExists(const std::string &name)
{
    return mAlbumCache.find(name) != mAlbumCache.end();
}
} // namespace db
} // namespace amp
