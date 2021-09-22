#include <amp/audio/player.hpp>
#include <iostream>

using namespace amp;

int main(int arg_n, char **args)
{
    int error = 0;
    bool quit = false;
    audio::Player player;

    if (arg_n < 2)
    {
        std::cerr << "usage: test_ffmpeg input.mp3" << std::endl;
        return -1;
    }

    error = player.loadAudioFile(args[1]);
    std::cout << "error = " << error << std::endl;
    if (error == 0)
    {
        std::cout << "File opened successfuly - ready for playing" << std::endl;
    }

    while (!quit)
    {
        std::string cmd;

        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "quit")
        {
            quit = true;
        }
        else if (cmd == "play")
        {
            player.play();
        }
        else if (cmd == "pause")
        {
            player.pause();
        }
        else if (cmd == "up")
        {
            player.increaseVolume(10.0f);
        }
        else if (cmd == "down")
        {
            player.decreaseVolume(10.f);
        }
    }
    return 0;
}
