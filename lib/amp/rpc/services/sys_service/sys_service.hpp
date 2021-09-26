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
#include <sys.grpc.pb.h>
#include <sys.pb.h>

namespace amp
{
namespace rpc
{
namespace service
{
class SysService : public ServiceData, public sys::Sys::Service
{
  public:
    SysService(std::shared_ptr<amp::Context> &ctx);

    grpc::Status AddFolder(grpc::ServerContext *ctx, const sys::FolderRequest *request, common::Empty *reply) override;
    grpc::Status Shutdown(grpc::ServerContext *ctx, const common::Empty *request, common::Empty *reply) override;
};
} // namespace service
} // namespace rpc
} // namespace amp
