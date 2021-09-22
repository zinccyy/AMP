#pragma once

#include <amp/db/database_manager.hpp>
#include <amp/audio/player.hpp>
#include <amp/folder_hash.hpp>

// stl
#include <list>
#include <string>
#include <unordered_map>

namespace amp
{
// context contains all data necessarry for music playing - database API, paths to the music folders etc.
class Context
{
  public:
    // init members
    Context();

    // loads all needed data and initializes the context
    int load();

    // adds the search path and scans the path for new music and fills the database
    int addSearchPath(const std::string &path);

    // rescan search path -> doesn't care for the folder already existing
    int rescanSearchPath(const std::string &path);
    int rescanSearchPath(const std::list<FolderHash> &folders);

    // folders getter
    std::vector<FolderHash> &getFolders()
    {
        return mFolders;
    }

  private:
    // manages database data and converts rows recieved to cxx struct types
    db::DatabaseManager mDatabase;

    // folders containing music - to be scanned recursively
    std::vector<FolderHash> mFolders;

    // music / audio player
    audio::Player mAudioPlayer;
};
} // namespace amp
