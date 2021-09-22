#include <amp/context.hpp>
#include <amp/log.hpp>
#include <amp/utils/path.hpp>

// db data
#include <amp/db/database_manager.hpp>
#include <amp/db/scheme/artist.hpp>
#include <amp/db/scheme/genre.hpp>
#include <amp/db/scheme/track.hpp>

// taglib
#include <exception>
#include <pqxx/result.hxx>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// stl
#include <list>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>

namespace amp
{
// creates a cover in the album folder directory from a track tag
static int generateAlbumCover(const TagLib::ID3v2::Tag *tag, const std::string &output_fpath);
// thread function for checking changes in a folder - once the folder contents have changed -> the folder should be rescanned and changes should be added
static void checkForFolderChanges(Context *ctx, FolderHash *folder);
// init members
Context::Context()
{
}
// loads all needed data and initializes the context
int Context::load()
{
    int error = 0;

    try
    {
        mDatabase.initConnection("AMP", "amp_admin", "amp_admin");
    }
    catch (const std::exception &e)
    {
        AMP_LOG_ERR("error connecting to the database: %s", e.what());
        return -1;
    }

    error = mDatabase.loadCache();
    if (error != 0)
    {
        return -1;
    }

    AMP_LOG_INF("starting threads for monitoring folder changes");

    // start threads to monitor changes on directories
    for (auto &folder : mFolders)
    {
        AMP_LOG_INF("starting a thread for \'%s\' folder", folder.Path.c_str());
        std::thread(checkForFolderChanges, this, &folder).detach();
    }
    return error;
}
// adds the search path and scans the path for new music and fills the database
int Context::addSearchPath(const std::string &path)
{
    if (std::find_if(mFolders.begin(), mFolders.end(), [&](const auto &f1) { return f1.Path == path; }) != mFolders.end())
    {
        // folder already in the cache path -> return error status
        return -1;
    }

    // add the new path to folders - storing into the config once the server shuts down
    mFolders.push_back(FolderHash{
        .Path = path,
        .Hash = utils::path::calculateHashForDirectory(path),
    });

    // automatically run changes checker for the folder
    AMP_LOG_INF("starting a thread for monitoring changes for directory \'%s\'", mFolders.back().Path.c_str());
    std::thread(checkForFolderChanges, this, &mFolders.back()).detach();

    return rescanSearchPath(path);
}
// rescan search path -> doesn't care for the folder already existing
int Context::rescanSearchPath(const std::string &path)
{
    int error = 0;

    // temp list for all file names gathered in the new directory
    std::list<std::string> file_paths;

    // snprintf queries into the buffer
    char query_buffer[2048] = {0};

    // traverse all directories
    error = utils::path::collectFiles(path, file_paths, true);
    if (error)
    {
        return error;
    }

    mDatabase.startTransaction();

    std::list<std::string> queried_paths = mDatabase.getRows<std::string>("SELECT track_file_path from track", [](const pqxx::row &row) { return row[0].as<std::string>(); });

    // no error -> add all files to the database
    for (auto &fpath : file_paths)
    {
        // file already exists in the DB - skip
        if (std::find(queried_paths.begin(), queried_paths.end(), fpath) != queried_paths.end())
        {
            continue;
        }

        TagLib::FileRef file(fpath.c_str());
        auto mpeg = (TagLib::MPEG::File *)file.file();
        auto id3v2_tag = mpeg->ID3v2Tag();

        // gather single track info and store data into the database
        std::string artist = file.tag()->artist().toCString();
        std::string title = file.tag()->title().toCString();
        std::string album = file.tag()->album().toCString();
        std::string genre = file.tag()->genre().toCString();
        std::string comment = file.tag()->comment().toCString();
        const auto track_number = file.tag()->track();
        const auto year = file.tag()->year();
        auto length = 0, sample_rate = 0, bitrate = 0, channels = 0;

        // sql query pre-changes

        if (file.audioProperties())
        {
            length = file.audioProperties()->lengthInSeconds();
            sample_rate = file.audioProperties()->sampleRate();
            bitrate = file.audioProperties()->bitrate();
            channels = file.audioProperties()->channels();
        }

        if (!mDatabase.artistExists(artist))
        {
            AMP_LOG_DBG("artist = %s", artist.c_str());
            error = mDatabase.insertArtist(db::scheme::Artist{
                .Name = artist,
            });
            if (error != 0)
            {
                AMP_LOG_INF("unable to insert artist \'%s\' into the DB", artist.c_str());
            }
        }

        if (!mDatabase.genreExists(genre))
        {
            error = mDatabase.insertGenre(db::scheme::Genre{
                .Name = genre,
            });
            if (error != 0)
            {
                AMP_LOG_INF("unable to insert genre \'%s\' into the DB", genre.c_str());
            }
        }

        if (!mDatabase.albumExists(album))
        {
            // generate cover
            std::string folder_path = utils::path::getDirectoryFromFilePath(fpath);
#ifdef _WIN32
            const std::string output_fpath = folder_path + "\\cover.jpg";
#else
            const std::string output_fpath = folder_path + "/cover.jpg";
#endif
            error = generateAlbumCover(id3v2_tag, output_fpath);
            if (error != 0)
            {
                // unable to generate cover
                AMP_LOG_ERR("unable to generate album cover for the album \'%s\'", album.c_str());
            }

            error = mDatabase.insertAlbum(
                db::scheme::Album{
                    .Name = album,
                    .FolderPath = folder_path,
                    .CoverPath = output_fpath,
                    .Year = year,
                },
                artist, genre);
            if (error != 0)
            {
                AMP_LOG_INF("unable to insert album \'%s\' into the DB", album.c_str());
            }
        }

        error = mDatabase.insertTrack(
            db::scheme::Track{
                .Name = title,
                .FilePath = fpath,
                .LengthInSeconds = length,
                .Number = track_number,
                .Comment = comment,
                .Bitrate = bitrate,
                .SampleRate = sample_rate,
                .Channels = channels,
            },
            album);
        if (error != 0)
        {
            AMP_LOG_INF("unable to insert track \'%s\' into the DB", title.c_str());
        }
    }

    mDatabase.commitTransaction();

    return error;
}
// creates a cover in the album folder directory from a track tag
static int generateAlbumCover(const TagLib::ID3v2::Tag *tag, const std::string &output_fpath)
{
    int error = 0;

    // temp struct for the image file saving
    struct ImageFile : TagLib::File
    {
      public:
        ImageFile(const char *fname) : TagLib::File(fname)
        {
        }

        TagLib::Tag *tag() const
        {
            return nullptr;
        }

        TagLib::AudioProperties *audioProperties() const
        {
            return nullptr;
        }

        bool save()
        {
            return true;
        }

        ~ImageFile()
        {
        }
    };

    // create the file
    std::ofstream create_file(output_fpath);

    if (create_file.is_open())
    {
        // close created file
        create_file.close();

        // open for reading with TagLib inherited data
        ImageFile output(output_fpath.c_str());

        if (tag)
        {
            auto frames = tag->frameListMap()["APIC"];
            if (frames.isEmpty())
            {
                AMP_LOG_INF("File has no built-in image");
            }
            else
            {
                // TODO: add support for all frames depending on types later -> for now just save the first frame as the cover (front)
                auto frame = frames[0];
                auto pic_frame = (TagLib::ID3v2::AttachedPictureFrame *)frame;

                // output the picture() vector into the file
                output.writeBlock(pic_frame->picture());
                output.save();
            }
        }
        else
        {
            AMP_LOG_ERR("invalid id3v2 tag...");
            error = -1;
        }
    }
    else
    {
        AMP_LOG_ERR("unable to create the file %s", output_fpath.c_str());
        error = -1;
    }

    return error;
}
// thread function for checking changes in a folder - once the folder contents have changed -> the folder should be rescanned and changes should be added
static void checkForFolderChanges(Context *ctx, FolderHash *folder)
{
    std::size_t hash = 0;
    int error = 0;

    while (true)
    {
        // first sleep for a period of time - hash has just been calculated for the given folder
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(30s);

        // after the break -> recalculate hash and if different rescan the directory and set the new hash
        hash = utils::path::calculateHashForDirectory(folder->Path);
        if (hash != folder->Hash)
        {
            AMP_LOG_INF("contents of the folder \'%s\' changed.. rescanning the folder for new data", folder->Path.c_str());
            folder->Hash = hash;
            error = ctx->rescanSearchPath(folder->Path);
            if (error)
            {
                AMP_LOG_ERR("error while rescanning folder \'%s\'", folder->Path.c_str());
                return;
            }
        }
        else
        {
            AMP_LOG_INF("no changes for folder \'%s\'", folder->Path.c_str());
        }
    }
}
} // namespace amp
