#ifndef VIDEO_H
#define VIDEO_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}
#include <SDL.h>
#include <string>
#include "Window.h"
#include "FPSOverlay.h"
#include <cstdint>

class Video {
public:
    Video(SDL_Renderer* renderer, const std::string& path);
    ~Video();
    bool open();
    void update(float dt);
    void render(int x, int y, int w, int h);
    bool isFinished() const;
    void run(Window& window, FPSOverlay& fpsOverlay, int x, int y, int w, int h);

private:
    SDL_Renderer* renderer;
    std::string path;

    AVFormatContext* fmtCtx;
    AVCodecContext* codecCtx;      // video codec context
    AVCodecContext* audioCtx;      // audio codec context
    const AVCodec* codec;         // video codec
    const AVCodec* audioCodec;    // audio codec
    int vidStreamIndex;
    int audioStreamIndex;

    AVFrame* frame;
    AVFrame* frameYUV;
    AVPacket* packet;
    SwsContext* swsCtx;
    SwrContext* swrCtx;

    SDL_Texture* texture;

    uint8_t* audioBuffer;       // temporary buffer for converted PCM (legacy)
    int audioBufferSize;        // size of audioBuffer in bytes
    int audioBufferPos;         // read position (if used)

    int texW;
    int texH;

    bool finished;
    double frameDelay;
    double accum;

    bool audioEnabled;

    // new fields used by Video.cpp sync logic
    bool audioStarted;          // whether we've started queueing audio
    uint32_t audioStartTicks;   // SDL_GetTicks() when audio started (ms)
    uint64_t audioTotalQueued;  // total bytes queued since start

    // helper
    static double avFramePtsToSeconds(AVFrame* f, AVStream* st);
};

#endif
