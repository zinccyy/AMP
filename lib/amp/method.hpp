#pragma once

#include <amp/context.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace amp
{
struct Method
{
    // method name
    std::string mName;

    // call function for a method - methods should implement this function
    const virtual json call(Context *ctx, const json &params, bool &success) = 0;
};
} // namespace amp
