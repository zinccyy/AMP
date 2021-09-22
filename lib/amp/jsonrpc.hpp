#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace amp
{
namespace jsonrpc
{
// creates a simple request json object
const json createRequest(const std::string &method, const json &params, int id);
// create simple result object
const json createResult(const json &result, int id);
// create an error result object
const json createError(int code, const std::string &message, const json &data, int id);
} // namespace jsonrpc
} // namespace amp
