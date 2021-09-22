#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>

namespace amp
{
// temp struct - for each folder calculate it's hash for later use
struct FolderHash
{
    std::string Path;
    std::size_t Hash;
};
// json conversions
void to_json(json &j, const FolderHash &fh);
void from_json(const json &j, FolderHash &fh);
} // namespace amp
