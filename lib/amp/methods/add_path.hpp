#pragma once

#include <amp/method.hpp>
#include <amp/jsonrpc.hpp>

#include <iostream>

namespace amp
{
namespace method
{
struct AddPath : public Method
{
    // init members
    AddPath()
    {
        mName = "add_path";
    }

    // inherit from amp::Module
    const json call(Context *ctx, const json &params, bool &success)
    {
        json j;
        int error = 0;

        error = ctx->addSearchPath(params["path"]);

        j["status"] = error;

        success = error == 0;
        return j;
    }
};
} // namespace method
} // namespace amp
