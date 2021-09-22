#include <fstream>
#include <iostream>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

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

static int generateAlbumCover(const char *input_fn, const char *output_fn)
{
    int error = 0;

    TagLib::FileRef file(input_fn);

    std::ofstream create_file(output_fn);

    if (!file.isNull() && create_file.is_open())
    {
        // close created file
        create_file.close();

        // open for reading with TagLib inherited data
        ImageFile output(output_fn);

        auto mpeg = (TagLib::MPEG::File *)file.file();
        auto tag = mpeg->ID3v2Tag();

        if (tag)
        {
            auto frames = tag->frameListMap()["Path"];
            if (frames.isEmpty())
            {
                std::cout << "File has no built-in image" << std::endl;
            }
            else
            {
                // extract frames and save to a file
                auto frame = frames[0];
                auto pic_frame = (TagLib::ID3v2::AttachedPictureFrame *)frame;

                // output the picture() vector into the file
                output.writeBlock(pic_frame->picture());
                output.save();
            }
        }
        else
        {
            std::cerr << "File has no id3v2 tag.. exiting" << std::endl;
            error = -1;
        }
    }
    else
    {
        std::cerr << "Unable to open file.." << std::endl;
        error = -1;
    }

    return error;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: test_taglib source_file output_file" << std::endl;
        return -1;
    }

    const char *input = argv[1];
    const char *output = argv[2];

    return generateAlbumCover(input, output);
}
