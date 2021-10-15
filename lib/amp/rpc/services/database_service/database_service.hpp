#pragma once

// lib
#include <amp/rpc/services/service_data.hpp>

// std
#include <iostream>
#include <memory>
#include <string>

// grpc
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// generated headers
#include <database.grpc.pb.h>
#include <database.pb.h>

namespace amp
{
namespace rpc
{
namespace service
{
class DatabaseService final : public ServiceData, public database::Database::Service
{
  public:
    DatabaseService(std::shared_ptr<amp::Context> &ctx);

    grpc::Status GetArtists(grpc::ServerContext *ctx, const common::Empty *request, database::ArtistList *reply) override;
    grpc::Status GetAlbums(grpc::ServerContext *ctx, const common::Empty *request, database::AlbumList *reply) override;
    grpc::Status GetGenres(grpc::ServerContext *ctx, const common::Empty *request, database::GenreList *reply) override;

    grpc::Status GetAlbumCover(grpc::ServerContext *ctx, const database::AlbumRequest *request, grpc::ServerWriter<database::ImageChunk> *reply) override;
};
} // namespace service
} // namespace rpc
} // namespace amp
