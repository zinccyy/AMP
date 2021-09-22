#include <iostream>
#include <queue>

// ffmpeg
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
// sdl2
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mutex.h>
#include <SDL_stdinc.h>
#undef main
}

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

struct PacketQueue
{
    PacketQueue()
    {
        Mutex = SDL_CreateMutex();
        Cond = SDL_CreateCond();
    }

    ~PacketQueue()
    {
        SDL_DestroyMutex(Mutex);
        SDL_DestroyCond(Cond);
    }

    void addPacket(AVPacket *packet)
    {
        AVPacket *clone = av_packet_clone(packet);

        SDL_LockMutex(Mutex);

        // update packet queue
        Packets.push(clone);
        Size += clone->size;

        // signal
        SDL_CondSignal(Cond);

        SDL_UnlockMutex(Mutex);
    }

    int getPacket(AVPacket *packet, int block)
    {
        int ret_val = 0;

        SDL_LockMutex(Mutex);

        for (;;)
        {
            if (!Packets.empty())
            {
                packet = Packets.front();
                Packets.pop();
                Size -= packet->size;
                ret_val = 1;
                break;
            }
            else if (!block)
            {
                ret_val = 0;
                break;
            }
            else
            {
                SDL_CondWait(Cond, Mutex);
            }
        }
        SDL_UnlockMutex(Mutex);

        return ret_val;
    }

    std::queue<AVPacket *> Packets;
    int Size;
    SDL_mutex *Mutex;
    SDL_cond *Cond;
};

// global for now
PacketQueue audio_q;
int audio_frame_count = 0;
bool quit = false;

void outputFrame(AVCodecContext *codec_ctx, AVFrame *frame)
{
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
    // printf("audio_frame n:%d nb_samples:%d pts:%s\n", audio_frame_count++, frame->nb_samples, av_ts2timestr(frame->pts, &codec_ctx->time_base));
}

int decodeAudioPacket(AVCodecContext *codec_ctx, AVPacket *packet, Uint8 *buffer, size_t size)
{
    int error = 0;
    AVFrame *frame = av_frame_alloc();

    error = avcodec_send_packet(codec_ctx, packet);
    if (error < 0)
    {
        std::cout << "avcodec_send_packet() failed" << std::endl;
        return error;
    }

    while (error >= 0)
    {
        error = avcodec_receive_frame(codec_ctx, frame);
        if (error < 0)
        {
            if (error == AVERROR_EOF || error == AVERROR(EAGAIN))
            {
                av_frame_free(&frame);
                return 0;
            }

            std::cerr << "avcodec_receive_frame() error" << std::endl;
            av_frame_free(&frame);
            return -1;
        }
        // got the frame -> output it
        outputFrame(codec_ctx, frame);

        av_frame_unref(frame);
    }

    av_frame_free(&frame);
    return error;
}

int audioDecode(AVCodecContext *codec_ctx, uint8_t *buf, size_t buf_size)
{
    AVPacket pkt;
    uint8_t *audio_pkt_data = NULL;
    int audio_pkt_size = 0;
    AVFrame frame;

    int len1, data_size = 0;

    for (;;)
    {
        while (audio_pkt_size > 0)
        {
            int got_frame = 0;
            // len1 = avcodec_decode_audio4(codec_ctx, &frame, &got_frame, &pkt);
            if (len1 < 0)
            {
                /* if error, skip frame */
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame)
            {
                data_size = av_samples_get_buffer_size(NULL, codec_ctx->channels, frame.nb_samples, codec_ctx->sample_fmt, 1);
                memcpy(buf, frame.data[0], data_size);
            }
            if (data_size <= 0)
            {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }

        if (quit)
        {
            return -1;
        }

        if (audio_q.getPacket(&pkt, 1) < 0)
        {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }

    return 0;
}

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    AVCodecContext *codec_ctx = (AVCodecContext *)userdata;
    // int len1 = 0, audio_size = 0;

    uint8_t audio_buf[MAX_AUDIO_FRAME_SIZE * 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;

    while (len > 0)
    {
        // store as much as you can into the stream -> decode a packet -> store frames data into the buffer
    }
    // while (len > 0)
    // {
    //     if (audio_buf_index >= audio_buf_size)
    //     {
    //         /* We have already sent all our data; get more */
    //         audio_size = audioDecode(codec_ctx, audio_buf, sizeof(audio_buf));
    //         if (audio_size < 0)
    //         {
    //             // error -> silence
    //             audio_buf_size = 1024;
    //             memset(audio_buf, 0, audio_buf_size);
    //         }
    //         else
    //         {
    //             audio_buf_size = audio_size;
    //         }
    //         audio_buf_index = 0;
    //     }
    //     len1 = audio_buf_size - audio_buf_index;
    //     if (len1 > len)
    //     {
    //         len1 = len;
    //     }
    //     memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
    //     len -= len1;
    //     stream += len1;
    //     audio_buf_index += len1;
    // }
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

    // SDL data
    SDL_AudioSpec wanted_spec = {0}, spec = {0};
    SDL_AudioDeviceID audio_dev = 0;
    SDL_Event event;

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

    // init SDL
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        std::cerr << "Could not initialize SDL - " << SDL_GetError() << std::endl;
        return -1;
    }

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

    wanted_spec.freq = codec_ctx->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = codec_ctx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;
    wanted_spec.callback = audioCallback;
    wanted_spec.userdata = codec_ctx;

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audio_dev < 2)
    {
        std::cerr << "SDL_OpenAudioDevice() error" << std::endl;
        return -1;
    }

    std::cout << "Starting to play sound" << std::endl;
    // this crap starts playing audio -> starts calling the callback function
    SDL_PauseAudioDevice(audio_dev, 0);

    // alloc needed structs for rendering audio
    packet = av_packet_alloc();
    frame = av_frame_alloc();

    sample_format = codec_ctx->sample_fmt;
    n_channels = codec_ctx->channels;
    int sample_rate = codec_ctx->sample_rate;

    std::printf("sample_format: %d, n_channels: %d, sample_rate: %d\n", sample_format, n_channels, sample_rate);

    while (av_read_frame(format_ctx, packet) == 0 && !quit)
    {
        // std::cout << "packet duration = " << (float)pkt.duration * audio_stream->time_base.num / audio_stream->time_base.den << std::endl;
        if (packet->stream_index == audio_stream_idx)
        {
            // decode audio packet
            // error = decodeAudioPacket(codec_ctx, packet);

            // add audio packet to queue for rendering to output once callback called
            // audio_q.addPacket(packet);
        }

        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            quit = true;
            break;
        }
        av_packet_unref(packet);
    }

    while (!quit)
    {
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            quit = true;
            break;
        }
    }

    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    return 0;
}
