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
#include <player.grpc.pb.h>
#include <player.pb.h>

namespace amp
{
namespace rpc
{
namespace service
{
class PlayerService final : public ServiceData, public player::Player::Service
{
  public:
    PlayerService(std::shared_ptr<amp::Context> &ctx);

    // RPCs
    grpc::Status PlaySong(grpc::ServerContext *context, const player::SongRequest *request, player::ActionStatus *reply) override;

    grpc::Status Play(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply) override;
    grpc::Status Pause(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply) override;
    grpc::Status Stop(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply) override;
};
} // namespace service
} // namespace rpc
} // namespace amp
