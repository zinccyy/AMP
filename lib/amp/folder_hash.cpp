#include <amp/folder_hash.hpp>

namespace amp
{
void to_json(json &j, const FolderHash &fh)
{
    j = json{
        {"path", fh.Path},
        {"hash", fh.Hash},
    };
}

void from_json(const json &j, FolderHash &fh)
{
    j.at("path").get_to(fh.Path);
    j.at("hash").get_to(fh.Hash);
}
} // namespace amp
