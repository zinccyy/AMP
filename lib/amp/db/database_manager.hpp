#pragma once

#include <amp/db/scheme/album.hpp>
#include <amp/db/scheme/artist.hpp>
#include <amp/db/scheme/genre.hpp>
#include <amp/db/scheme/track.hpp>
#include <amp/db/pqxx_fix.hpp>

#include <pqxx/pqxx>

#include <list>
#include <string>

namespace amp
{
namespace db
{
template <typename T> using FromRowTransform = T(const pqxx::row &row);
class DatabaseManager
{
  public:
    // setup members
    DatabaseManager();

    // initializes the connection with the database
    void initConnection(const char *dbName, const char *username, const char *password);

    // loads the current database table data for quicker access later on
    int loadCache();

    // starts a new transaction
    void startTransaction();

    // commits the current transaction
    void commitTransaction();

    // executes a query and returns the list of wanted types
    template <typename T> std::list<T> getRows(const char query[], FromRowTransform<T> transform)
    {
        std::list<T> rows;

        auto result = mTx->exec(query);

        for (auto row : result)
        {
            rows.push_back(transform(row));
        }
        return rows;
    }

    // executes a query and returns a single row value
    template <typename T> T getRow(const char query[], FromRowTransform<T> transform)
    {
        auto result = mTx->exec1(query);
        return transform(result);
    }

    // some specific functions for inserting cxx structs into the database
    int insertArtist(const scheme::Artist &artist);
    int insertGenre(const scheme::Genre &genre);
    int insertAlbum(const scheme::Album &album, const std::string &artist, const std::string &genre);
    int insertTrack(const scheme::Track &track, const std::string &album);

    // check wether the value exists in the database
    bool artistExists(const std::string &name);
    bool genreExists(const std::string &name);
    bool albumExists(const std::string &name);

  private:
    // connection to the DB
    std::unique_ptr<pqxx::connection> mConn;

    // current transaction
    std::unique_ptr<pqxx::work> mTx;

    // cache for artist names
    std::unordered_map<std::string, int> mArtistCache;

    // cache for album titles
    std::unordered_map<std::string, int> mAlbumCache;

    // cache for genres
    std::unordered_map<std::string, int> mGenreCache;

    // last field ID used in the database
    unsigned int mArtistID;
    unsigned int mAlbumID;
    unsigned int mGenreID;
    unsigned int mTrackID;
};
} // namespace db
} // namespace amp
