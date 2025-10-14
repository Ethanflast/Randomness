#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include "Window.h"
#include "Intro.h"

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) > 0)
        std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl;
    if (!(IMG_Init(IMG_INIT_PNG)))
        std::cout << "IMG_Init failed: " << SDL_GetError() << std::endl;
    if (TTF_Init() == -1)
        std::cout << "TTF_Init failed: " << TTF_GetError() << std::endl;

    RenderWindow window("GAME v1.0", 1280, 720);

    TTF_Font* font = TTF_OpenFont("JetBrainsMono-Regular.ttf", 48);
    if (font == NULL)
        std::cout << "Font load failed: " << TTF_GetError() << std::endl;

    Intro intro(window.getRenderer(), font, "Made by someone", "Welcome to the slayer", 1280, 720, 1.2f, 4.0f);
    intro.start();

    bool gameRunning = true;
    SDL_Event event;
    Uint32 last = SDL_GetTicks();

    while (gameRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                gameRunning = false;
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;

        intro.update(dt);

        SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 0, 255);
        window.clear();
        intro.render();
        window.display();

        if (intro.isDone())
            gameRunning = false;
    }

    TTF_CloseFont(font);
    window.cleanUp();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
