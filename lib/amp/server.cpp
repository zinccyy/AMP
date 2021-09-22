#include <amp/log.hpp>
#include <amp/server.hpp>
#include <amp/jsonrpc.hpp>
#include <amp/folder_hash.hpp>
#include <amp/utils/path.hpp>

// methods
#include <amp/methods/calc.hpp>
#include <amp/methods/add_path.hpp>

// libs
#include <nlohmann/json.hpp>
#include <string.h>
#include <uv.h>

// stl
#include <string>
#include <iostream>
#include <fstream>

namespace amp
{
// simplify json access
using json = nlohmann::json;
// temp struct for async data reading
struct ReadData
{
    // pointer to the running server
    Server *mServer;

    // string for storing the buffered data
    std::string mData;
};
// temp struct for async data writing
struct WriteData
{
    // write struct
    uv_write_t mWrite;

    // buffer
    uv_buf_t mBuffer;
};
// static functions required for libuv request reading and response writing
static void newConnectionCallback(uv_stream_t *server, int status);
static void allocBufferCallback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void readDataCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
static void writeDataCallback(uv_write_t *req, int status);
static void sigintCallback(uv_signal_t *handle, int signum);
static const json callModule(Server *server, const json &rpc);
// init members
Server::Server() : mPort(3000)
{
    // load config.json at the start
    std::ifstream json_config;
    json j;

    json_config.open("config.json");

    if (json_config.is_open())
    {
        // read file into a string and parse needed data
        std::string contents((std::istreambuf_iterator<char>(json_config)), std::istreambuf_iterator<char>());
        try
        {
            j = json::parse(contents);
            mContext.getFolders() = j["folders"].get<std::vector<FolderHash>>();
        }
        catch (json::parse_error &ex)
        {
            AMP_LOG_ERR("parse error at byte %llu", ex.byte);
        }

        json_config.close();
    }

    mContext.load();
}
// parse arguments for server configuration
int Server::parseArgs(int arg_n, char **args)
{
    int error = 0;

    return error;
}
// main function - loads the modules, configures the server and runs until interrupted
int Server::run()
{
    int error = 0;

    // preload methods
    mModules.push_back(new method::Calc());
    mModules.push_back(new method::AddPath());

    struct sockaddr_in addr = {0};
    uv_signal_t sigint;

    // initialize loop and run
    mLoop = uv_default_loop();

    // configure tcp
    uv_tcp_init(mLoop, &mTCPServer);
    uv_ip4_addr("0.0.0.0", mPort, &addr);

    uv_tcp_bind(&mTCPServer, (const struct sockaddr *)&addr, 0);
    mTCPServer.data = this;
    error = uv_listen((uv_stream_t *)&mTCPServer, 128, newConnectionCallback);

    if (error)
    {
        return error;
    }

    // configure SIGINT
    sigint.data = this;
    uv_signal_init(mLoop, &sigint);
    uv_signal_start(&sigint, sigintCallback, SIGINT);

    AMP_LOG_INF("starting the server loop...");

    // run main loop
    error = uv_run(mLoop, UV_RUN_DEFAULT);
    if (error)
    {
        return error;
    }

    return error;
}
void Server::close()
{
    // save configuration for the next start - json
    std::ofstream json_config;
    json j;

    j["folders"] = mContext.getFolders();

    json_config.open("config.json");

    if (json_config.is_open())
    {
        // output json and close file
        json_config << j.dump(2);

        json_config.close();
    }

    // stop the loop
    uv_stop(mLoop);
}
// static functions required for libuv
static void newConnectionCallback(uv_stream_t *server, int status)
{
    Server *srv = (Server *)server->data;
    if (status < 0)
    {
        AMP_LOG_ERR("error occured: %d -> %s", status, uv_strerror(status));
        return;
    }

    // allocate struct for the new client
    uv_tcp_t *client = new uv_tcp_t();
    uv_tcp_init(server->loop, client);

    client->data = new ReadData();
    ReadData *read_data = (ReadData *)client->data;

    read_data->mServer = srv;
    read_data->mData = std::string();

    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        AMP_LOG_INF("accepted new connection");
        AMP_LOG_INF("starting reading from client");
        uv_read_start((uv_stream_t *)client, allocBufferCallback, readDataCallback);
    }
    else
    {
        uv_close((uv_handle_t *)client, nullptr);
    }
}
static void allocBufferCallback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)new char[2048];
    buf->len = 2048;
}
static void readDataCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    ReadData *read_data = (ReadData *)stream->data;
    Server *server = (Server *)read_data->mServer;
    std::string *data = &read_data->mData;

    AMP_LOG_INF("got new request");

    if (nread > 0)
    {
        // read 'nread' bytes -> append them to the current read string for a client
        *data = std::string(buf->base, 0, nread);

        json j, response;
        try
        {
            j = json::parse(*data);
            AMP_LOG_INF("data recieved: \"%s\"", j.dump().c_str());
            response = callModule(server, j);

            // write a response
            WriteData *wd = new WriteData;
            const std::string val = response.dump();

            wd->mBuffer.base = _strdup(val.c_str());
            wd->mBuffer.len = strlen(wd->mBuffer.base);
            wd->mBuffer.base[wd->mBuffer.len] = 0;

            AMP_LOG_INF("starting writing to the client");
            uv_write(&wd->mWrite, stream, &wd->mBuffer, 1, writeDataCallback);
        }
        catch (json::parse_error &ex)
        {
            std::cerr << "parse error at byte " << ex.byte << std::endl;
        }

        // free allocated data struct
        delete read_data;
    }
    else
    {
        if (nread == UV_EOF)
        {
            uv_close((uv_handle_t *)stream, nullptr);
        }
    }

    free(buf->base);
}
static void writeDataCallback(uv_write_t *req, int status)
{
    WriteData *wd = (WriteData *)req;

    if (status)
    {
        // error writing data
        AMP_LOG_ERR("libuv error (%d): %s", status, uv_strerror(status));
    }

    // free write data temp struct
    delete wd->mBuffer.base;
    delete wd;
}
static void sigintCallback(uv_signal_t *handle, int signum)
{
    Server *server = (Server *)handle->data;
    server->close();
}
static const json callModule(Server *server, const json &rpc)
{
    json response;
    Method *method_ptr = nullptr;
    const std::string &method = rpc["method"];
    bool success = false;

    for (auto &mod : server->mModules)
    {
        if (mod->mName == method)
        {
            method_ptr = mod;
            break;
        }
    }

    if (method_ptr)
    {
        const json output = method_ptr->call(&server->mContext, rpc["params"], success);
        response = success ? jsonrpc::createResult(output, rpc["id"]) : jsonrpc::createError(-32603, "Internal error", output, rpc["id"]);
    }
    else
    {
        response = amp::jsonrpc::createError(-32601, "Method not found", json{}, rpc["id"]);
    }
    return response;
}
} // namespace amp
