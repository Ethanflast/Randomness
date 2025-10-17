#include "Video.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

static SDL_AudioDeviceID s_audioDevice = 0;

Video::Video(SDL_Renderer* renderer, const std::string& path) {
    this->renderer = renderer;
    this->path = path;
    fmtCtx = NULL;
    codecCtx = NULL;
    audioCtx = NULL;
    codec = NULL;
    audioCodec = NULL;
    vidStreamIndex = -1;
    audioStreamIndex = -1;
    frame = NULL;
    frameYUV = NULL;
    packet = NULL;
    swsCtx = NULL;
    swrCtx = NULL;
    texture = NULL;
    audioBuffer = NULL;
    audioBufferSize = 0;
    audioBufferPos = 0;
    texW = 0;
    texH = 0;
    finished = false;
    frameDelay = 0.0;
    accum = 0.0;
    audioEnabled = false;
    audioStarted = false;
    audioStartTicks = 0;
    audioTotalQueued = 0;
}

Video::~Video() {
    if (s_audioDevice != 0) {
        SDL_CloseAudioDevice(s_audioDevice);
        s_audioDevice = 0;
    }
    if (texture) SDL_DestroyTexture(texture);
    if (swsCtx) sws_freeContext(swsCtx);
    if (frameYUV) {
        if (frameYUV->data[0]) av_freep(&frameYUV->data[0]);
        av_frame_free(&frameYUV);
    }
    if (frame) av_frame_free(&frame);
    if (packet) av_packet_free(&packet);
    if (codecCtx) avcodec_free_context(&codecCtx);
    if (audioCtx) avcodec_free_context(&audioCtx);
    if (swrCtx) swr_free(&swrCtx);
    if (fmtCtx) avformat_close_input(&fmtCtx);
    if (audioBuffer) free(audioBuffer);
}

bool Video::open() {
    if (avformat_open_input(&fmtCtx, path.c_str(), NULL, NULL) != 0) {
        std::cerr << "avformat_open_input failed\n";
        return false;
    }
    if (avformat_find_stream_info(fmtCtx, NULL) < 0) {
        std::cerr << "avformat_find_stream_info failed\n";
        return false;
    }
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) vidStreamIndex = i;
        else if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) audioStreamIndex = i;
    }
    if (vidStreamIndex == -1) {
        std::cerr << "no video stream\n";
        return false;
    }
    AVCodecParameters* codecpar = fmtCtx->streams[vidStreamIndex]->codecpar;
    codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        std::cerr << "video decoder not found\n";
        return false;
    }
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) return false;
    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0) return false;
    if (avcodec_open2(codecCtx, codec, NULL) < 0) return false;
    frame = av_frame_alloc();
    frameYUV = av_frame_alloc();
    texW = codecCtx->width;
    texH = codecCtx->height;
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, texW, texH, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) return false;
    av_image_fill_arrays(frameYUV->data, frameYUV->linesize, buffer, AV_PIX_FMT_YUV420P, texW, texH, 1);
    swsCtx = sws_getContext(texW, texH, codecCtx->pix_fmt, texW, texH, AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR, NULL, NULL, NULL);
    packet = av_packet_alloc();
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, texW, texH);
    if (!texture) return false;
    audioEnabled = false;
    if (audioStreamIndex != -1) {
        audioCodec = avcodec_find_decoder(fmtCtx->streams[audioStreamIndex]->codecpar->codec_id);
        if (!audioCodec) audioEnabled = false;
        else {
            audioCtx = avcodec_alloc_context3(audioCodec);
            if (!audioCtx) return false;
            if (avcodec_parameters_to_context(audioCtx, fmtCtx->streams[audioStreamIndex]->codecpar) < 0) return false;
            if (avcodec_open2(audioCtx, audioCodec, NULL) < 0) return false;
            swrCtx = swr_alloc();
            if (!swrCtx) return false;
            av_opt_set_chlayout(swrCtx, "in_chlayout", &audioCtx->ch_layout, 0);
            av_opt_set_chlayout(swrCtx, "out_chlayout", &audioCtx->ch_layout, 0);
            av_opt_set_int(swrCtx, "in_sample_rate", audioCtx->sample_rate, 0);
            av_opt_set_int(swrCtx, "out_sample_rate", audioCtx->sample_rate, 0);
            av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", audioCtx->sample_fmt, 0);
            av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
            if (swr_init(swrCtx) < 0) {
                swr_free(&swrCtx);
                audioEnabled = false;
            } else audioEnabled = true;
        }
    }
    if (audioEnabled) {
        audioBufferSize = 192000;
        audioBuffer = (uint8_t*)malloc(audioBufferSize);
        if (!audioBuffer) audioEnabled = false;
        else {
            SDL_AudioSpec want;
            SDL_AudioSpec obtained;
            SDL_zero(want);
            SDL_zero(obtained);
            want.freq = audioCtx->sample_rate;
            want.format = AUDIO_S16SYS;
            want.channels = (Uint8)audioCtx->ch_layout.nb_channels;
            want.samples = 4096;
            s_audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &obtained, 0);
            if (s_audioDevice == 0) {
                std::cout << "SDL_OpenAudioDevice failed: " << SDL_GetError() << std::endl;
                audioEnabled = false;
            } else {
                SDL_PauseAudioDevice(s_audioDevice, 0);
            }
        }
    }
    AVRational fpsr = fmtCtx->streams[vidStreamIndex]->r_frame_rate;
    frameDelay = (fpsr.num != 0 && fpsr.den != 0) ? (double)fpsr.den / fpsr.num : 1.0 / 25.0;
    accum = 0.0;
    finished = false;
    return true;
}

double Video::avFramePtsToSeconds(AVFrame* f, AVStream* st) {
    int64_t pts = f->best_effort_timestamp;
    if (pts == AV_NOPTS_VALUE) pts = f->pts;
    return pts * av_q2d(st->time_base);
}

void Video::update(float dt) {
    (void)dt;
}

void Video::render(int x, int y, int w, int h) {
    if (!texture) return;
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w == 0 ? texW : w;
    dst.h = h == 0 ? texH : h;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

bool Video::isFinished() const { return finished; }

void Video::run(Window& window, FPSOverlay& fpsOverlay, int x, int y, int w, int h) {
    const int SCREEN_FPS = 60;
    const int SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;
    Uint32 last = SDL_GetTicks();
    SDL_Event event;
    bool running = true;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.0f;
    bool fadingIn = true;
    float fpsTimer = 0.0f;
    int framesCounted = 0;
    AVFrame* vframe = av_frame_alloc();
    AVFrame* aframe = av_frame_alloc();
    double audio_clock_pts = 0.0; // media time of last decoded audio frame (seconds)
    double audioStartWallClock = 0.0; // wall-clock time when audio pts==0
    bool audioPtsStarted = false;
    while (running && !finished) {
        Uint32 frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) { if (event.type == SDL_QUIT) running = false; }
        bool gotVideoFrame = false;
        while (!gotVideoFrame && running && !finished) {
            int ret = av_read_frame(fmtCtx, packet);
            if (ret < 0) {
                avcodec_send_packet(codecCtx, NULL);
                while (avcodec_receive_frame(codecCtx, vframe) == 0) {
                    sws_scale(swsCtx, vframe->data, vframe->linesize, 0, codecCtx->height, frameYUV->data, frameYUV->linesize);
                    SDL_UpdateYUVTexture(texture, NULL, frameYUV->data[0], frameYUV->linesize[0], frameYUV->data[1], frameYUV->linesize[1], frameYUV->data[2], frameYUV->linesize[2]);
                    gotVideoFrame = true;
                    av_frame_unref(vframe);
                    break;
                }
                finished = true;
                break;
            }
            if (packet->stream_index == audioStreamIndex && audioEnabled) {
                if (avcodec_send_packet(audioCtx, packet) >= 0) {
                    while (avcodec_receive_frame(audioCtx, aframe) == 0) {
                        // compute pts (seconds) for this audio frame
                        double thisAudioPts = avFramePtsToSeconds(aframe, fmtCtx->streams[audioStreamIndex]);
                        // resample
                        uint8_t** dst_data = NULL;
                        int dst_linesize = 0;
                        int max_out_samples = av_rescale_rnd(swr_get_delay(swrCtx, audioCtx->sample_rate) + aframe->nb_samples,
                                                             audioCtx->sample_rate, audioCtx->sample_rate, AV_ROUND_UP);
                        if (av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, audioCtx->ch_layout.nb_channels,
                                                               max_out_samples, AV_SAMPLE_FMT_S16, 0) < 0) {
                            av_frame_unref(aframe);
                            continue;
                        }
                        int out_samples = swr_convert(swrCtx, dst_data, max_out_samples, (const uint8_t**)aframe->data, aframe->nb_samples);
                        if (out_samples > 0) {
                            int out_bytes = av_samples_get_buffer_size(NULL, audioCtx->ch_layout.nb_channels, out_samples, AV_SAMPLE_FMT_S16, 1);
                            if (out_bytes > 0 && s_audioDevice != 0) {
                                if (!audioStarted) {
                                    audioStarted = true;
                                    audioStartTicks = SDL_GetTicks();
                                    audioTotalQueued = 0;
                                    SDL_ClearQueuedAudio(s_audioDevice);
                                }
                                if (SDL_QueueAudio(s_audioDevice, dst_data[0], out_bytes) == 0) {
                                    audioTotalQueued += (uint64_t)out_bytes;
                                }
                                // update audio pts timing anchor
                                audio_clock_pts = thisAudioPts;
                                if (!audioPtsStarted) {
                                    audioPtsStarted = true;
                                    audioStartWallClock = (double)SDL_GetTicks() / 1000.0 - audio_clock_pts;
                                }
                            }
                        }
                        av_freep(&dst_data[0]);
                        av_freep(&dst_data);
                        av_frame_unref(aframe);
                    }
                }
                av_packet_unref(packet);
            } else if (packet->stream_index == vidStreamIndex) {
                if (avcodec_send_packet(codecCtx, packet) >= 0) {
                    while (avcodec_receive_frame(codecCtx, vframe) == 0) {
                        sws_scale(swsCtx, vframe->data, vframe->linesize, 0, codecCtx->height, frameYUV->data, frameYUV->linesize);
                        SDL_UpdateYUVTexture(texture, NULL, frameYUV->data[0], frameYUV->linesize[0], frameYUV->data[1], frameYUV->linesize[1], frameYUV->data[2], frameYUV->linesize[2]);
                        double pts_seconds = avFramePtsToSeconds(vframe, fmtCtx->streams[vidStreamIndex]);
                        // compute desired wall-clock time when this video frame should be shown
                        double desiredShowTime = pts_seconds; // media time
                        double currentWall = (double)SDL_GetTicks() / 1000.0;
                        double audioWall = 0.0;
                        bool haveAudioAnchor = audioPtsStarted;
                        if (haveAudioAnchor) audioWall = audioStartWallClock + audio_clock_pts;
                        // If we have audio anchor, wait until audio progressed to the video's pts
                        int waitLoops = 0;
                        while (running && waitLoops < 500) {
                            currentWall = (double)SDL_GetTicks() / 1000.0;
                            if (haveAudioAnchor) {
                                // audioWall is wall-clock time corresponding to audio_clock_pts; need to recompute audioWall based on latest audio_clock_pts
                                // (audio_clock_pts is updated when decoding audio frames)
                                audioWall = audioStartWallClock + audio_clock_pts;
                                if (currentWall >= (audioStartWallClock + pts_seconds)) break;
                            } else {
                                // fallback: use wall-clock vs media time by comparing media pts against time since start
                                // assume video start was when run() began: approximate start time
                                static double runStartWall = 0.0;
                                if (runStartWall == 0.0) runStartWall = (double)SDL_GetTicks() / 1000.0;
                                if (currentWall >= runStartWall + pts_seconds) break;
                            }
                            SDL_Delay(1);
                            waitLoops++;
                        }
                        gotVideoFrame = true;
                        av_frame_unref(vframe);
                        break;
                    }
                }
                av_packet_unref(packet);
            } else {
                av_packet_unref(packet);
            }
        }
        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 0, 255);
        window.clear();
        render(x, y, w, h);
        if (fadingIn) {
            fadeTimer += dt;
            int alpha = 255 - (int)(255.0f * (fadeTimer / fadeDuration));
            if (alpha < 0) alpha = 0;
            SDL_SetRenderDrawBlendMode(window.getRenderer(), SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 0, alpha);
            SDL_Rect fullScreen = {0, 0, 1280, 720};
            SDL_RenderFillRect(window.getRenderer(), &fullScreen);
            if (fadeTimer >= fadeDuration) fadingIn = false;
        }
        framesCounted++;
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) {
            fpsOverlay.update(framesCounted / fpsTimer);
            fpsTimer = 0.0f;
            framesCounted = 0;
        }
        fpsOverlay.render();
        window.display();
        Uint32 frameTicks = SDL_GetTicks() - frameStart;
        if (frameTicks < SCREEN_TICK_PER_FRAME) SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
    }
    av_frame_free(&vframe);
    av_frame_free(&aframe);
    if (s_audioDevice != 0) {
        SDL_Delay(100);
        SDL_CloseAudioDevice(s_audioDevice);
        s_audioDevice = 0;
    }
}
