#include "Window.h"
#include <iostream>

Window::Window(const char* title, int w, int h)
    : window(nullptr), renderer(nullptr)
{
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    if (!window) std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
}

Window::~Window() {
    cleanUp();
}

bool Window::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl; return false; }
    if (!(IMG_Init(IMG_INIT_PNG))) { std::cout << "IMG_Init failed: " << SDL_GetError() << std::endl; return false; }
    if (TTF_Init() == -1) { std::cout << "TTF_Init failed: " << TTF_GetError() << std::endl; return false; }
    return true;
}

SDL_Texture* Window::loadTexture(const std::string& path) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) std::cout << "Failed to load texture: " << SDL_GetError() << std::endl;
    return tex;
}

void Window::clear() { SDL_RenderClear(renderer); }
void Window::render(SDL_Texture* tex) { SDL_RenderCopy(renderer, tex, nullptr, nullptr); }
void Window::display() { SDL_RenderPresent(renderer); }

void Window::cleanUp() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;
}
