#include "database_service.hpp"
#include <amp/log.hpp>

#include <string>
#include <fstream>

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
grpc::Status DatabaseService::GetAlbumCover(grpc::ServerContext *ctx, const database::AlbumRequest *request, grpc::ServerWriter<database::ImageChunk> *reply)
{
    AMP_LOG_INF("DatabaseService::GetAlbumCover() called");

    char query_buffer[2048] = {0};

    // 1: get album cover path
    auto path_conversion = [](const pqxx::row &row) { return row[0].as<std::string>(); };
    snprintf(query_buffer, sizeof(query_buffer),
             "SELECT album_cover from album natural join artist natural join genre where album_name = \'%s\' and artist_name = \'%s\' and genre_name = \'%s\'",
             request->name().c_str(), request->artist().c_str(), request->genre().c_str());

    mContext->mDatabase.startTransaction();

    const auto cover_path = mContext->mDatabase.getRow<std::string>(query_buffer, path_conversion);

    mContext->mDatabase.commitTransaction();

    // 2: load file into a buffer
    std::ifstream file(cover_path, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        AMP_LOG_ERR("unable to open file %s", cover_path.c_str());
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Unable to find album cover for the wanted album");
    }
    else
    {
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // 3: send chunks of data
        const auto chunk_size = 1024;
        size_t iter = 0;

        while (iter < buffer.size())
        {
            const auto dist = buffer.size() - iter;
            const auto chunk_len = (dist <= chunk_size) ? dist : chunk_size;

            // add temp chunk
            database::ImageChunk chunk;
            chunk.set_data(std::string{buffer.begin() + iter, buffer.begin() + iter + chunk_len});

            reply->Write(chunk);

            iter += chunk_len;
        }

        file.close();
    }
    return grpc::Status::OK;
}
} // namespace service
} // namespace rpc
} // namespace amp
