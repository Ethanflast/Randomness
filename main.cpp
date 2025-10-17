#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include "Window.h"
#include "Intro.h"
#include "Video.h"
#include "FPSOverlay.h"

int SDL_main(int argc, char* argv[]) {
    Window window("Randomness", 1280, 720);
    if (!window.initSDL()) return 1;

    FPSOverlay fpsOverlay;
    fpsOverlay.init(window.getRenderer(), "JetBrainsMono-Regular.ttf", 20);

    TTF_Font* font = TTF_OpenFont("JetBrainsMono-Regular.ttf", 48);
    if (!font) return 1;

    Intro intro(window.getRenderer(), font, "Made by someone", "Welcome to the slayer", 1280, 720);
    intro.start();
    intro.run(window, fpsOverlay);

    Video video(window.getRenderer(), "mambo.mp4");
    if (video.open()) video.run(window, fpsOverlay, 0, 0, 1280, 720);

    fpsOverlay.free();
    TTF_CloseFont(font);
    window.cleanUp();
    return 0;
}
