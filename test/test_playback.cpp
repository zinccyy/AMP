#include <iostream>
#include <queue>
#include <csignal>

// ffmpeg
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

bool quit = false;

void sigintHandler(int sig)
{
    quit = true;
}

void dataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    // write data to the device
    AVAudioFifo *buffer = (AVAudioFifo *)pDevice->pUserData;

    av_audio_fifo_read(buffer, &pOutput, frameCount);
}

int main(int arg_n, char **args)
{
    int error = 0;
    AVFormatContext *format_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    AVCodec *codec = nullptr;
    AVStream *audio_stream = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    AVSampleFormat sample_format = AV_SAMPLE_FMT_NONE;

    int n_channels = 0;
    int audio_stream_idx = 0;

    std::string file_path;
    std::string full_path;

    if (arg_n < 2)
    {
        std::cerr << "usage: test_ffmpeg input.mp3" << std::endl;
        return -1;
    }

    // create full path
    file_path = args[1];
    full_path = "file:" + file_path;

    // open file
    std::cout << "attempting to open " << file_path << std::endl;
    if (avformat_open_input(&format_ctx, full_path.c_str(), NULL, NULL) < 0)
    {
        std::cerr << "avformat_open_input() error" << std::endl;
        return -1;
    }
    std::cout << "opened file " << file_path << std::endl;
    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        std::cerr << "avformat_find_stream_info() error" << std::endl;
        return -1;
    }

    signal(SIGINT, sigintHandler);

    // get audio stream
    audio_stream_idx = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_stream_idx < 0)
    {
        std::cerr << "av_find_best_stream() error" << std::endl;
        return -1;
    }
    audio_stream = format_ctx->streams[audio_stream_idx];

    // setup codec context
    codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
    if (!codec)
    {
        std::cerr << "avcodec_find_decoder() error" << std::endl;
        return -1;
    }
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        std::cerr << "avcodec_alloc_context3() error" << std::endl;
        return -1;
    }
    if (avcodec_parameters_to_context(codec_ctx, audio_stream->codecpar) < 0)
    {
        std::cerr << "avcodec_parameters_to_context() failed" << std::endl;
        return -1;
    }
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        std::cerr << "avcodec_open2() failed" << std::endl;
        return -1;
    }

    // print format
    av_dump_format(format_ctx, 0, full_path.c_str(), 0);

    // alloc needed structs for playing back audio
    packet = av_packet_alloc();
    frame = av_frame_alloc();

    SwrContext *resample_ctx =
        swr_alloc_set_opts(nullptr, audio_stream->codecpar->channel_layout, AV_SAMPLE_FMT_FLT, audio_stream->codecpar->sample_rate, audio_stream->codecpar->channel_layout,
                           (AVSampleFormat)audio_stream->codecpar->format, audio_stream->codecpar->sample_rate, 0, nullptr);
    AVAudioFifo *audio_buffer = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, audio_stream->codecpar->channels, 1);

    sample_format = codec_ctx->sample_fmt;
    n_channels = codec_ctx->channels;
    int sample_rate = codec_ctx->sample_rate;

    while (av_read_frame(format_ctx, packet) == 0)
    {
        if (packet->stream_index == audio_stream_idx)
        {
            // add data to buffer
            error = avcodec_send_packet(codec_ctx, packet);
            if (error < 0)
            {
                if (error != AVERROR(EAGAIN))
                {
                    std::cerr << "avcodec_send_packet() error" << std::endl;
                    return -1;
                }
            }
            while ((error = avcodec_receive_frame(codec_ctx, frame)) == 0)
            {
                AVFrame *resampled_frame = av_frame_alloc();

                resampled_frame->sample_rate = frame->sample_rate;
                resampled_frame->channel_layout = frame->channel_layout;
                resampled_frame->channels = frame->channels;
                resampled_frame->format = AV_SAMPLE_FMT_FLT;

                error = swr_convert_frame(resample_ctx, resampled_frame, frame);
                if (error < 0)
                {
                    std::cerr << "swr_convert_frame() error" << std::endl;
                    return -1;
                }

                // store resampled frames inside a buffer
                av_audio_fifo_write(audio_buffer, (void **)resampled_frame->data, resampled_frame->nb_samples);

                av_frame_free(&resampled_frame);
                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    }

    // play back the audio
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;                      // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = audio_stream->codecpar->channels; // Set to 0 to use the device's native channel count.
    config.sampleRate = audio_stream->codecpar->sample_rate;     // Set to 0 to use the device's native sample rate.
    config.dataCallback = dataCallback;                          // This function will be called when miniaudio needs more data.
    config.pUserData = audio_buffer;                             // Can be accessed from the device object (device.pUserData).

    ma_device device;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS)
    {
        std::cerr << "ma_device_init() error" << std::endl;
        return -1; // Failed to initialize the device.
    }

    ma_device_start(&device); // The device is sleeping by default so you'll need to start it manually.

    // Do something here. Probably your program's main loop.

    while (av_audio_fifo_size(audio_buffer) && !quit)
        ;

    ma_device_uninit(&device); // This will stop the device so no need to do that manually.
    av_audio_fifo_free(audio_buffer);
    swr_free(&resample_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    return 0;
}
