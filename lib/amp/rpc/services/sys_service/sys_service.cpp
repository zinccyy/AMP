#include "sys_service.hpp"
#include <amp/rpc/server.hpp>

namespace amp
{
namespace rpc
{
namespace service
{
SysService::SysService(std::shared_ptr<amp::Context> &ctx)
{
    mContext = ctx;
}
grpc::Status SysService::AddFolder(grpc::ServerContext *ctx, const sys::FolderRequest *request, common::Empty *reply)
{
    grpc::Status status(grpc::Status::OK);

    auto error = mContext->addSearchPath(request->path());
    if (error)
    {
        status = grpc::Status(grpc::StatusCode::INTERNAL, "unable to add provided path to the search folders");
    }

    return status;
}
grpc::Status SysService::Shutdown(grpc::ServerContext *ctx, const common::Empty *request, common::Empty *reply)
{
    (void)ctx;
    (void)request;
    (void)reply;

    mContext->mServer->shutdown();
    return grpc::Status::OK;
}
} // namespace service
} // namespace rpc
} // namespace amp
