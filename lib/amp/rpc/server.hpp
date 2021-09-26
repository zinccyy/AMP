#pragma once

#include <amp/context.hpp>

#include <grpcpp/grpcpp.h>

#include <memory>

namespace amp
{
namespace rpc
{
class Server
{
  public:
    Server();

    void parseArguments(int arg_n, char **argv);

    void loadConfiguration();

    void run();

    void shutdown();

  private:
    // helpers to separate tasks into functions
    void mInitContext();
    void mInitServices();

    // configuration
    int mPort;

    // all data used in the server
    std::shared_ptr<amp::Context> mContext;

    // grpc data
    std::unique_ptr<grpc::Server> mServer;

    // services
    std::list<std::shared_ptr<grpc::Service>> mServices;
};
} // namespace rpc
} // namespace amp
