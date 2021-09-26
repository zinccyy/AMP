#include "player_service.hpp"
#include <amp/log.hpp>

namespace amp
{
namespace rpc
{
namespace service
{
PlayerService::PlayerService(std::shared_ptr<amp::Context> &ctx)
{
    mContext = ctx;
}
grpc::Status PlayerService::PlaySong(grpc::ServerContext *context, const player::SongRequest *request, player::ActionStatus *reply)
{
    grpc::Status status(grpc::Status::OK);
    char query_buffer[2048] = {0};
    const std::string &song = request->song();

    AMP_LOG_DBG("PlayerService::PlaySong() called");

    // got song name -> get path from DB

    return status;
}
grpc::Status PlayerService::Play(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("PlayerService::Play() called");

    return status;
}
grpc::Status PlayerService::Pause(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("PlayerService::Pause() called");

    return status;
}
grpc::Status PlayerService::Stop(grpc::ServerContext *context, const common::Empty *request, player::ActionStatus *reply)
{
    grpc::Status status(grpc::Status::OK);

    AMP_LOG_DBG("PlayerService::Stop() called");

    return status;
}
} // namespace service
} // namespace rpc
} // namespace amp
