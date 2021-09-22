#include <amp/audio/player.hpp>
#include <amp/log.hpp>

#include <iostream>
#include <string>

extern "C"
{
// ffmpeg
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
// miniaudio
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
}

namespace amp
{
namespace audio
{
// callback used for passing audio data to the miniaudio device
static void playerDataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);
// init all members
Player::Player()
{
    // ffmpeg
    mAudioFifo = nullptr;

    // start with maximized volume
    mVolume = 100.f;
    mState = PlayerState::None;

    // miniaudio
    mMADevice = nullptr;
    mMAConfig = nullptr;
}
// loads the file
int Player::loadAudioFile(const char *fpath)
{
    int error = 0;
    int av_error = 0;
    AVFormatContext *format_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    AVCodec *codec = nullptr;
    AVStream *audio_stream = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    SwrContext *resample_ctx = nullptr;
    AVAudioFifo *audio_fifo = nullptr;

    int audio_stream_idx = 0;

    std::string file_path;
    std::string full_path;

    // create full path
    file_path = fpath;
    full_path = "file:" + file_path;

    // open file
    if (avformat_open_input(&format_ctx, full_path.c_str(), NULL, NULL) < 0)
    {
        AMP_LOG_ERR("avformat_open_input() error");
        goto error_out;
    }
    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        AMP_LOG_ERR("avformat_find_stream_info() error");
        goto error_out;
    }

    // get audio stream
    audio_stream_idx = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_stream_idx < 0)
    {
        AMP_LOG_ERR("av_find_best_stream() error");
        goto error_out;
    }
    audio_stream = format_ctx->streams[audio_stream_idx];

    // setup codec context
    codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
    if (!codec)
    {
        AMP_LOG_ERR("avcodec_find_decoder() error");
        goto error_out;
    }
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        AMP_LOG_ERR("avcodec_alloc_context3() error");
        goto error_out;
    }
    if (avcodec_parameters_to_context(codec_ctx, audio_stream->codecpar) < 0)
    {
        AMP_LOG_ERR("avcodec_parameters_to_context() error");
        goto error_out;
    }
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        AMP_LOG_ERR("avcodec_open2() error");
        goto error_out;
    }

    // print format
    // av_dump_format(format_ctx, 0, full_path.c_str(), 0);

    // alloc needed structs for playing back audio
    packet = av_packet_alloc();
    frame = av_frame_alloc();

    resample_ctx = swr_alloc_set_opts(nullptr, audio_stream->codecpar->channel_layout, AV_SAMPLE_FMT_FLT, audio_stream->codecpar->sample_rate,
                                      audio_stream->codecpar->channel_layout, (AVSampleFormat)audio_stream->codecpar->format, audio_stream->codecpar->sample_rate, 0, nullptr);
    if (!resample_ctx)
    {
        AMP_LOG_ERR("swr_alloc_set_opts() error");
        goto error_out;
    }
    audio_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, audio_stream->codecpar->channels, 1);
    if (!audio_fifo)
    {
        AMP_LOG_ERR("av_audio_fifo_alloc() error");
        goto error_out;
    }

    mAudioFifo = audio_fifo;

    while (av_read_frame(format_ctx, packet) == 0)
    {
        if (packet->stream_index == audio_stream_idx)
        {
            // add data to buffer
            av_error = avcodec_send_packet(codec_ctx, packet);
            if (av_error < 0)
            {
                if (av_error != AVERROR(EAGAIN))
                {
                    goto error_out;
                }
            }
            while ((av_error = avcodec_receive_frame(codec_ctx, frame)) == 0)
            {
                AVFrame *resampled_frame = av_frame_alloc();

                resampled_frame->sample_rate = frame->sample_rate;
                resampled_frame->channel_layout = frame->channel_layout;
                resampled_frame->channels = frame->channels;
                resampled_frame->format = AV_SAMPLE_FMT_FLT;

                av_error = swr_convert_frame(resample_ctx, resampled_frame, frame);
                if (av_error < 0)
                {
                    goto error_out;
                }

                // store resampled frames inside a buffer
                av_audio_fifo_write(mAudioFifo, (void **)resampled_frame->data, resampled_frame->nb_samples);

                av_frame_free(&resampled_frame);
                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    }

    // configure device for playback
    mMAConfig = new ma_device_config;
    mMADevice = new ma_device;

    *mMAConfig = ma_device_config_init(ma_device_type_playback);
    mMAConfig->playback.format = ma_format_f32;
    mMAConfig->playback.channels = audio_stream->codecpar->channels;
    mMAConfig->sampleRate = audio_stream->codecpar->sample_rate;
    mMAConfig->dataCallback = playerDataCallback;
    mMAConfig->pUserData = mAudioFifo;

    if (ma_device_init(NULL, mMAConfig, mMADevice) != MA_SUCCESS)
    {
        AMP_LOG_ERR("ma_device_init() error");
        goto error_out;
    }

    // playback device initialized and is ready for playing audio using the given callback - free all non needed structs
    mState = PlayerState::Paused;

    goto out;
error_out:
    AMP_LOG_ERR("unable to open file %s", file_path.c_str());
    error = -1;

out:
    // free no longer needed structs
    swr_free(&resample_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return error;
}
// audio player actions
void Player::play()
{
    ma_device_start(mMADevice);
    mState = PlayerState::Playing;
}
void Player::pause()
{
    ma_device_stop(mMADevice);
    mState = PlayerState::Paused;
}
void Player::close()
{
    destroy();
    mState = PlayerState::Closed;
}
// volume control
void Player::setVolume(float volume)
{
    mVolume = volume;
    mUpdateVolume();
}
void Player::increaseVolume(float inc)
{
    if (mVolume + inc > 100.f)
    {
        mVolume = 100.f;
    }
    else
    {
        mVolume += inc;
    }
    mUpdateVolume();
}
void Player::decreaseVolume(float dec)
{
    if (mVolume - dec < 0.0f)
    {
        mVolume = 0.0f;
    }
    else
    {
        mVolume -= dec;
    }
    mUpdateVolume();
}
// getters
float Player::getVolume() const
{
    return mVolume;
}
PlayerState Player::getState() const
{
    return mState;
}
// destroys the current context (fifo buffer and uninitializes the current device)
void Player::destroy()
{
    if (mAudioFifo)
    {
        av_audio_fifo_free(mAudioFifo);
        mAudioFifo = nullptr;
    }
    if (mMADevice)
    {
        ma_device_uninit(mMADevice);

        delete mMADevice;
        delete mMAConfig;

        mMADevice = nullptr;
        mMAConfig = nullptr;
    }

    mState = PlayerState::None;
}
void Player::mUpdateVolume()
{
    ma_device_set_master_volume(mMADevice, mVolume / 100.f);
}
Player::~Player()
{
    destroy();
}
// callback used for passing audio data to the miniaudio device
static void playerDataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    // write data to the device
    AVAudioFifo *buffer = (AVAudioFifo *)pDevice->pUserData;

    // transfer data from buffer to the needed output
    av_audio_fifo_read(buffer, &pOutput, frameCount);
}
} // namespace audio
} // namespace amp
