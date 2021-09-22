#pragma once

#include <amp/method.hpp>

#include <iostream>

namespace amp
{
namespace method
{
struct Play : public Method
{
    // init members
    Play()
    {
        mName = "play";
    }

    // inherit from amp::Module
    const json call(Context *ctx, const json &params, bool &success)
    {
        json j;

        const std::string &song_name = j["song"];

        // ctx->playSong(song_name); or something like that

        return j;
    }
};
} // namespace method
} // namespace amp
