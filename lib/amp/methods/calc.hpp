#pragma once

#include <amp/method.hpp>

#include <iostream>

namespace amp
{
namespace method
{
struct Calc : public Method
{
    // init members
    Calc()
    {
        mName = "calc";
    }

    // inherit from amp::Module
    const json call(Context *ctx, const json &params, bool &success)
    {
        json j;

        // form the output object
        j["z"] = (int)params["x"] + (int)params["y"];

        // set if the operation is successful or not
        success = true;

        // return the response
        return j;
    }
};
} // namespace method
} // namespace amp
