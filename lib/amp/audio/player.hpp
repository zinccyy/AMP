#pragma once

// predefine ffmpeg and miniaudio used structs and include headers in the cxx file rather than in hxx
// necessarry because of libuv linking issues when miniaudio included - libs define same structs
struct AVAudioFifo;
struct ma_device_config;
struct ma_device;

namespace amp
{
namespace audio
{
enum class PlayerState
{
    None = 0,
    Playing,
    Paused,
    Closed, // equivalent to stop, named closed because the whole device and stream closes when transferring in this state
};
class Player
{
  public:
    Player();

    // loads the file
    int loadAudioFile(const char *fpath);

    // audio player actions
    void play();
    void pause();
    void close();

    // volume control
    void setVolume(float volume);
    void increaseVolume(float inc);
    void decreaseVolume(float dec);

    // getters
    float getVolume() const;
    PlayerState getState() const;
    bool isDone() const;

    // destroys the current context (fifo buffer and uninitializes the current device)
    void destroy();

    ~Player();

  private:
    // updates the device volume
    void mUpdateVolume();

    // ffmpeg data for decoding audio files
    AVAudioFifo *mAudioFifo;

    // miniaudio data for playback
    ma_device *mMADevice;
    ma_device_config *mMAConfig;

    // other data - volume, current position stuff etc.
    float mVolume;
    PlayerState mState;
};
} // namespace audio
} // namespace amp
