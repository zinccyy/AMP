#include "database_service.hpp"
#include <amp/log.hpp>

namespace amp
{
namespace rpc
{
namespace service
{
DatabaseService::DatabaseService(std::shared_ptr<amp::Context> &ctx)
{
    mContext = ctx;
}
grpc::Status DatabaseService::GetAlbums(grpc::ServerContext *ctx, const common::Empty *request, database::AlbumList *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("DatabaseService::GetAlbums() called");

    const char query[] = "SELECT album_name, artist_name, genre_name, album_folder_path, album_year FROM album NATURAL JOIN artist NATURAL JOIN genre";

    auto album_output_transform = [](const pqxx::row &row) {
        database::Album album;

        album.set_name(row[0].as<std::string>());
        album.set_artist(row[1].as<std::string>());
        album.set_genre(row[2].as<std::string>());
        album.set_folder(row[3].as<std::string>());
        album.set_year(row[4].as<std::string>());

        return album;
    };

    mContext->mDatabase.startTransaction();
    auto albums = mContext->mDatabase.getRows<database::Album>(query, album_output_transform);
    mContext->mDatabase.commitTransaction();

    for (auto album : albums)
    {
        auto new_album = reply->add_albums();
        *new_album = album;
    }

    return status;
}
grpc::Status DatabaseService::GetArtists(grpc::ServerContext *ctx, const common::Empty *request, database::ArtistList *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("DatabaseService::GetArtists() called");

    const char query[] = "SELECT artist_name, artist_image FROM artist";

    auto album_output_transform = [](const pqxx::row &row) {
        database::Artist artist;

        artist.set_name(row[0].as<std::string>());

        if (!row[1].is_null())
        {
            artist.set_cover_path(row[1].as<std::string>());
        }

        return artist;
    };

    mContext->mDatabase.startTransaction();
    auto artists = mContext->mDatabase.getRows<database::Artist>(query, album_output_transform);
    mContext->mDatabase.commitTransaction();

    for (auto artist : artists)
    {
        auto new_artist = reply->add_artists();
        *new_artist = artist;
    }

    return status;
}
grpc::Status DatabaseService::GetGenres(grpc::ServerContext *ctx, const common::Empty *request, database::GenreList *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("DatabaseService::GetGenres() called");

    const char query[] = "SELECT genre_name FROM genre";

    auto album_output_transform = [](const pqxx::row &row) {
        database::Genre genre;

        genre.set_name(row[0].as<std::string>());

        return genre;
    };

    mContext->mDatabase.startTransaction();
    auto genres = mContext->mDatabase.getRows<database::Genre>(query, album_output_transform);
    mContext->mDatabase.commitTransaction();

    for (auto genre : genres)
    {
        auto new_genre = reply->add_genres();
        *new_genre = genre;
    }

    return status;
}
} // namespace service
} // namespace rpc
} // namespace amp
