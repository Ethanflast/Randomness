#ifndef FPS_OVERLAY_H
#define FPS_OVERLAY_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class FPSOverlay {
public:
    FPSOverlay();
    ~FPSOverlay();
    bool init(SDL_Renderer* renderer, const std::string& fontPath, int fontSize);
    void update(float fps);
    void render();
    void free();
    void setPosition(int x, int y);
    void setColor(Uint8 r, Uint8 g, Uint8 b);
private:
    SDL_Renderer* mRenderer;
    TTF_Font* mFont;
    SDL_Texture* mTexture;
    SDL_Color mColor;
    int mWidth;
    int mHeight;
    int mX;
    int mY;
    std::string mLastText;
};
#endif
