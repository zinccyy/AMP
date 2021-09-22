#include <amp/jsonrpc.hpp>

namespace amp
{
namespace jsonrpc
{
// create a simple request json object
const json createRequest(const std::string &method, const json &params, int id)
{
    json j;

    j["jsonrpc"] = "2.0";
    j["method"] = method;
    j["params"] = params;
    j["id"] = id;

    return j;
}
// create simple result object
const json createResult(const json &result, int id)
{
    json j;

    j["jsonrpc"] = "2.0";
    j["result"] = result;
    j["id"] = id;

    return j;
}
// create an error result object
const json createError(int code, const std::string &message, const json &data, int id)
{
    json j;

    j["jsonrpc"] = "2.0";
    j["error"] = json{
        {"code", code},
        {"message", message},
        {"data", data},
    };
    j["id"] = id;

    return j;
}
} // namespace jsonrpc
} // namespace amp
