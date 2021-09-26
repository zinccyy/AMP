#include <amp/rpc/server.hpp>
#include <amp/log.hpp>

// services
#include <amp/rpc/services/database_service/database_service.hpp>
#include <amp/rpc/services/player_service/player_service.hpp>
#include <amp/rpc/services/sys_service/sys_service.hpp>

// grpc
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/grpcpp.h>

// libuv
#include <uv.h>

// std
#include <fstream>
#include <csignal>

namespace amp
{
namespace rpc
{
Server::Server() : mPort(3000)
{
    mContext = std::make_shared<amp::Context>(this);
}
void Server::parseArguments(int arg_n, char **argv)
{
    AMP_LOG_INF("parsing given app arguments");
}
void Server::loadConfiguration()
{
    // load configuration
    AMP_LOG_INF("loading server configuration");
    std::ifstream json_config;
    json j;

    json_config.open("config.json");

    if (json_config.is_open())
    {
        // read file into a string and parse needed data
        std::string contents((std::istreambuf_iterator<char>(json_config)), std::istreambuf_iterator<char>());

        // don't put inside try/catch -> propagate instead
        j = json::parse(contents);

        mContext->mFolders = j["folders"].get<std::vector<FolderHash>>();
        json_config.close();
    }
    else
    {
        throw std::runtime_error("Unable to open file config.json");
    }
}
void Server::mInitContext()
{
    AMP_LOG_INF("initializing server context");
    mContext->load();
}
void Server::mInitServices()
{
    AMP_LOG_INF("initializing server services");
    mServices = {
        std::make_shared<service::DatabaseService>(mContext),
        std::make_shared<service::PlayerService>(mContext),
        std::make_shared<service::SysService>(mContext),
    };
}
void Server::run()
{
    AMP_LOG_INF("running the server");

    // initialize context
    mInitContext();

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;

    const std::string addr_uri = "localhost:" + std::to_string(mPort);

    builder.AddListeningPort(addr_uri, grpc::InsecureServerCredentials());

    // init services
    mInitServices();

    // register services
    for (auto &service : mServices)
    {
        builder.RegisterService(service.get());
    }

    mServer = std::unique_ptr<grpc::Server>(builder.BuildAndStart());

    AMP_LOG_INF("server started at %s", addr_uri.c_str());

    mServer->Wait();
}
void Server::shutdown()
{
    // store configuration
    std::ofstream json_config;
    json j;

    AMP_LOG_INF("storing data into config.json");

    j["folders"] = mContext->mFolders;

    json_config.open("config.json");

    if (json_config.is_open())
    {
        // output json and close file
        json_config << j.dump(2);
        json_config.close();
    }

    // close the server
    mServer->Shutdown();
}
} // namespace rpc
} // namespace amp
