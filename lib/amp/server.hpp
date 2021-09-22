#pragma once

#include <amp/method.hpp>
#include <uv.h>

#include <list>

namespace amp
{
struct Server
{
    // libuv loop
    uv_loop_t *mLoop;

    // use tcp server for requests
    uv_tcp_t mTCPServer;

    // port for the server
    int mPort;

    // list of modules
    std::list<Method *> mModules;

    // music data context
    Context mContext;

    // init members
    Server();

    // parse arguments for server configuration
    int parseArgs(int arg_n, char **args);

    // main function - loads the modules, configures the server and runs until interrupted
    int run();

    // close the server and output needed context data into the config file
    void close();
};
} // namespace amp
