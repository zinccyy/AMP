#include <amp.hpp>
#include <iostream>
#include <csignal>
using namespace amp;

rpc::Server server;

void sigintCallback(int sig);

int main(int arg_n, char **args)
{
    // setup CTRL + C callback
    signal(SIGINT, sigintCallback);

    try
    {
        server.parseArguments(arg_n, args);
        server.loadConfiguration();
        server.run();
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    AMP_LOG_INF("server closed");
    return 0;
}

void sigintCallback(int sig)
{
    server.shutdown();
}