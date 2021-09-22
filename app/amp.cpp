#include <amp.hpp>

int main(int arg_n, char **args)
{
    int error = 0;
    amp::Server server;

    error = server.parseArgs(arg_n, args);
    if (error)
    {
        return error;
    }

    return server.run();
}
