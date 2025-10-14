#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <SDL.h>
#include <string>
class VideoPlayer {
public:
    VideoPlayer(SDL_Renderer* renderer, const std::string& path);
    ~VideoPlayer();
    bool open();
    void update();
    void render(int x, int y, int w, int h);
    bool isFinished() const;
private:
    SDL_Renderer* renderer;
    std::string path;
    AVFormatContext* fmtCtx;
    AVCodecContext* codecCtx;
    const AVCodec* codec;
    int vidStreamIndex;
    AVFrame* frame;
    AVFrame* frameYUV;
    AVPacket* packet;
    SwsContext* swsCtx;
    SDL_Texture* texture;
    int texW;
    int texH;
    bool finished;
    double frameDelay;
    double accum;
};
#endif
