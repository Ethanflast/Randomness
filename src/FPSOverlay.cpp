#include "FPSOverlay.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <sstream>

FPSOverlay::FPSOverlay()
{
    mRenderer = nullptr;
    mFont = nullptr;
    mTexture = nullptr;
    mColor = { 255, 255, 255, 255 };
    mWidth = 0;
    mHeight = 0;
    mX = 8;
    mY = 8;
    mLastText = "";
}

FPSOverlay::~FPSOverlay()
{
    free();
}

bool FPSOverlay::init(SDL_Renderer* renderer, const std::string& fontPath, int fontSize)
{
    mRenderer = renderer;
    mFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    return mFont != nullptr;
}

void FPSOverlay::update(float fps)
{
    std::stringstream ss;
    ss << "FPS: " << (int)(fps + 0.5f);
    std::string newText = ss.str();
    if (newText == mLastText) return;
    mLastText = newText;

    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }

    if (mFont == nullptr || mRenderer == nullptr) {
        mWidth = 0;
        mHeight = 0;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(mFont, newText.c_str(), mColor);
    if (textSurface != nullptr) {
        mTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
        mWidth = textSurface->w;
        mHeight = textSurface->h;
        SDL_FreeSurface(textSurface);
    } else {
        mTexture = nullptr;
        mWidth = 0;
        mHeight = 0;
    }
}

void FPSOverlay::render()
{
    if (mTexture == nullptr) return;
    SDL_Rect dst = { mX, mY, mWidth, mHeight };
    SDL_RenderCopy(mRenderer, mTexture, nullptr, &dst);
}

void FPSOverlay::free()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    if (mFont != nullptr) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
    mLastText.clear();
}

void FPSOverlay::setPosition(int x, int y)
{
    mX = x;
    mY = y;
}

void FPSOverlay::setColor(Uint8 r, Uint8 g, Uint8 b)
{
    mColor = { r, g, b, 255 };
    mLastText.clear();
}
