#include "RunScene.h"
#include "Video.h"
#include <SDL.h>

class LTimer {
public:
    LTimer() { mStartTicks = 0; mPausedTicks = 0; mPaused = false; mStarted = false; }
    void start() { mStarted = true; mPaused = false; mStartTicks = SDL_GetTicks(); mPausedTicks = 0; }
    void stop() { mStarted = false; mPaused = false; mStartTicks = 0; mPausedTicks = 0; }
    Uint32 getTicks() const { if (!mStarted) return 0; if (mPaused) return mPausedTicks; return SDL_GetTicks() - mStartTicks; }
private:
    Uint32 mStartTicks;
    Uint32 mPausedTicks;
    bool mPaused;
    bool mStarted;
};

RunScene::RunScene(Window* window, FPSOverlay* fpsOverlay) {
    this->window = window;
    this->fpsOverlay = fpsOverlay;
}

void RunScene::runIntro(Intro* intro) {
    Uint32 last = SDL_GetTicks();
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;

        intro->update(dt);

        SDL_SetRenderDrawColor(window->getRenderer(), 0, 0, 0, 255);
        window->clear();
        intro->render();
        fpsOverlay->render();
        window->display();

        if (intro->isDone()) break;
    }
}

void RunScene::runVideo(Video* video, int x, int y, int w, int h) {
    Uint32 last = SDL_GetTicks();
    bool running = true;
    SDL_Event event;
    const int SCREEN_FPS = 60;
    const int SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;
    LTimer capTimer;

    while (running && !video->isFinished()) {
        capTimer.start();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;

        video->update(dt);

        SDL_SetRenderDrawColor(window->getRenderer(), 0, 0, 0, 255);
        window->clear();
        video->render(x, y, w, h);

        if (dt > 0.0f) fpsOverlay->update(1.0f / dt);
        fpsOverlay->render();
        window->display();

        int frameTicks = capTimer.getTicks();
        if (frameTicks < SCREEN_TICK_PER_FRAME) SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
    }
}
