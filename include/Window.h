#ifndef WINDOW_H
#define WINDOW_H
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>

class Window {
public:
    Window(const char* title, int w, int h);
    ~Window();

    bool initSDL();
    SDL_Texture* loadTexture(const std::string& path);
    void clear();
    void render(SDL_Texture* tex);
    void display();
    void cleanUp();

    SDL_Renderer* getRenderer() { return renderer; }
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};
#endif
