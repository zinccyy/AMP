#pragma once

#include <string>
#include <list>

namespace amp
{
namespace utils
{
namespace path
{
// recursively traverses from the starting directory and inserts all file names into the passed list
int collectFiles(const std::string &path, std::list<std::string> &filePaths, bool addAllFileTypes = false);
// calculates the hash for a given directory contents (files)
size_t calculateHashForDirectory(const std::string &path);
// extract directory name from the given path
std::string getDirectoryFromFilePath(const std::string &fpath);
// returns the file extension
std::string getExtensionFromFilePath(const std::string &fpath);
// checks if the file extension is an audio format
bool isAudioFileExtension(const std::string &ext);
} // namespace path
} // namespace utils
} // namespace amp
