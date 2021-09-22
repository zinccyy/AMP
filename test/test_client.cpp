#include <amp.hpp>
#include <string.h>
#include <uv.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

uv_loop_t *loop = NULL;

static void connect_cb(uv_connect_t *req, int status);
static void write_cb(uv_write_t *write, int status);
static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
static const json build_simple_req();

int main()
{
    int err = 0;
    uv_tcp_t client = {0};
    uv_connect_t conn = {0};
    struct sockaddr_in addr = {0};

    // make default loop
    loop = uv_default_loop();

    // bind and configure everything
    uv_tcp_init(loop, &client);
    uv_ip4_addr("127.0.0.1", 3000, &addr);

    err = uv_tcp_connect(&conn, &client, (const struct sockaddr *)&addr, connect_cb);

    // run
    err = uv_run(loop, UV_RUN_DEFAULT);

    // cleanup
    uv_loop_close(loop);
    return err;
}

void connect_cb(uv_connect_t *req, int status)
{
    AMP_LOG_INF("connected...");

    if (status < 0)
    {
        return;
    }

    json json_req = build_simple_req();
    std::string val = json_req.dump();
    const size_t len = val.size();

    std::cout << val << std::endl;

    // status = OK -> send data using TCP connection
    uv_buf_t *buf = new uv_buf_t;
    uv_write_t *write = new uv_write_t;

#ifdef WIN32
    buf->base = _strdup(val.c_str());
#else
    buf->base = strdup(strval);
#endif
    buf->len = len;

    uv_write(write, req->handle, buf, 1, write_cb);
}

void write_cb(uv_write_t *req, int status)
{
    if (status < 0)
    {
        return;
    }
    else
    {
        uv_read_start(req->handle, alloc_cb, read_cb);
        // uv_close((uv_handle_t *)req->handle, 0);
    }
}

void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[2048];
    buf->len = 2048;
}

void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            printf("DONE\n");
        }
        return;
    }
    else
    {
        if (nread > 0)
        {
            std::cout << nread << std::endl;
            buf->base[nread] = 0;
            printf("%s", buf->base);
            uv_close((uv_handle_t *)stream, 0);
        }
    }
}

const json build_simple_req()
{
    json j;

    j["x"] = 10;
    j["y"] = 20;

    return amp::jsonrpc::createRequest("calc", j, 1);
}
