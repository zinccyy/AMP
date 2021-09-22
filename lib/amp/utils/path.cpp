#include <amp/utils/path.hpp>

#include <cstddef>
#include <vcruntime.h>
#include <windows.h>

namespace amp
{
namespace utils
{
namespace path
{
// recursively traverses from the starting directory and inserts all file names into the passed list
int collectFiles(const std::string &path, std::list<std::string> &filePaths, bool addAllFileTypes)
{
    int error = 0;
#ifdef _WIN32
    WIN32_FIND_DATA fd_file;
    HANDLE h_find = NULL;
    std::string full_path;

    full_path = path + "\\*.*";

    if ((h_find = FindFirstFile(full_path.c_str(), &fd_file)) == INVALID_HANDLE_VALUE)
    {
        error = -1;
        return error;
    }

    do
    {
        if (strcmp(fd_file.cFileName, ".") != 0 && strcmp(fd_file.cFileName, "..") != 0)
        {
            full_path = path + "\\" + fd_file.cFileName;

            if ((fd_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                // file
                if (!addAllFileTypes)
                {
                    if (getExtensionFromFilePath(full_path) == "mp3")
                    {
                        filePaths.push_back(full_path);
                    }
                }
                else
                {
                    filePaths.push_back(full_path);
                }
            }
            else if (fd_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // dir
                utils::path::collectFiles(full_path, filePaths);
            }
        }
    } while (FindNextFile(h_find, &fd_file));

    FindClose(h_find);
#else
#endif

    return error;
}
size_t calculateHashForDirectory(const std::string &path)
{
    std::size_t hash = 0;

    std::list<std::string> files;
    std::hash<std::string> hash_func;

    utils::path::collectFiles(path, files);

    for (auto &file : files)
    {
        // TODO: see if this is OK - nope
        hash += hash_func(file);
    }

    return hash;
}
// extract directory name from the given path
std::string getDirectoryFromFilePath(const std::string &fpath)
{
    auto pos = fpath.find_last_of("/\\");
    return fpath.substr(0, pos);
}
// returns the file extension
std::string getExtensionFromFilePath(const std::string &fpath)
{
    return fpath.substr(fpath.find_last_of(".") + 1);
}
// checks if the file extension is an audio format
bool isAudioFileExtension(const std::string &ext)
{
    // TODO: add more later
    return ext == "mp3" || ext == "wav";
}
} // namespace path
} // namespace utils
} // namespace amp
