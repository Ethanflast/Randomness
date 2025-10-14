#include "Video.h"
VideoPlayer::VideoPlayer(SDL_Renderer* renderer, const std::string& path) {
    this->renderer = renderer;
    this->path = path;
    fmtCtx = NULL;
    codecCtx = NULL;
    codec = NULL;
    vidStreamIndex = -1;
    frame = NULL;
    frameYUV = NULL;
    packet = NULL;
    swsCtx = NULL;
    texture = NULL;
    texW = 0;
    texH = 0;
    finished = false;
    frameDelay = 0.0;
    accum = 0.0;
}
VideoPlayer::~VideoPlayer() {
    if (texture != NULL) SDL_DestroyTexture(texture);
    if (swsCtx != NULL) sws_freeContext(swsCtx);
    if (frameYUV != NULL) av_frame_free(&frameYUV);
    if (frame != NULL) av_frame_free(&frame);
    if (packet != NULL) av_packet_free(&packet);
    if (codecCtx != NULL) avcodec_free_context(&codecCtx);
    if (fmtCtx != NULL) avformat_close_input(&fmtCtx);
}
bool VideoPlayer::open() {
    if (avformat_open_input(&fmtCtx, path.c_str(), NULL, NULL) != 0) return false;
    if (avformat_find_stream_info(fmtCtx, NULL) < 0) return false;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vidStreamIndex = (int)i;
            break;
        }
    }
    if (vidStreamIndex == -1) return false;
    AVCodecParameters* codecpar = fmtCtx->streams[vidStreamIndex]->codecpar;
    const AVCodec* codecFound = avcodec_find_decoder(codecpar->codec_id);
    if (!codecFound) return false;
    codec = codecFound;
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) return false;
    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0) return false;
    if (avcodec_open2(codecCtx, codec, NULL) < 0) return false;
    frame = av_frame_alloc();
    frameYUV = av_frame_alloc();
    int width = codecCtx->width;
    int height = codecCtx->height;
    texW = width;
    texH = height;
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) return false;
    av_image_fill_arrays(frameYUV->data, frameYUV->linesize, buffer, AV_PIX_FMT_YUV420P, width, height, 1);
    swsCtx = sws_getContext(width, height, codecCtx->pix_fmt, width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
    packet = av_packet_alloc();
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) return false;
    AVRational fpsr = fmtCtx->streams[vidStreamIndex]->r_frame_rate;
    if (fpsr.num != 0 && fpsr.den != 0) frameDelay = (double)fpsr.den / (double)fpsr.num;
    else frameDelay = 1.0 / 25.0;
    accum = 0.0;
    finished = false;
    return true;
}
void VideoPlayer::update() {
    if (finished) return;
    static uint32_t lastTicks = SDL_GetTicks();
    uint32_t now = SDL_GetTicks();
    double dt = (now - lastTicks) / 1000.0;
    lastTicks = now;
    accum += dt;
    while (accum >= frameDelay) {
        accum -= frameDelay;
        bool frameDecoded = false;
        while (!frameDecoded) {
            if (av_read_frame(fmtCtx, packet) < 0) {
                finished = true;
                break;
            }
            if (packet->stream_index == vidStreamIndex) {
                int ret = avcodec_send_packet(codecCtx, packet);
                av_packet_unref(packet);
                if (ret < 0) continue;
                ret = avcodec_receive_frame(codecCtx, frame);
                if (ret == 0) {
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height, frameYUV->data, frameYUV->linesize);
                    SDL_UpdateYUVTexture(texture, NULL, frameYUV->data[0], frameYUV->linesize[0], frameYUV->data[1], frameYUV->linesize[1], frameYUV->data[2], frameYUV->linesize[2]);
                    frameDecoded = true;
                }
            } else {
                av_packet_unref(packet);
            }
        }
        if (finished) break;
    }
}
void VideoPlayer::render(int x, int y, int w, int h) {
    if (texture == NULL) return;
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w == 0 ? texW : w;
    dst.h = h == 0 ? texH : h;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}
bool VideoPlayer::isFinished() const {
    return finished;
}
