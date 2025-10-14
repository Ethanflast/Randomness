#include "Label.h"
Label::Label(SDL_Renderer* renderer, TTF_Font* font) {
    this->renderer = renderer;
    this->font = font;
    texture = NULL;
    dst.x = 0;
    dst.y = 0;
    dst.w = 0;
    dst.h = 0;
    posX = 0.0f;
    posY = 0.0f;
    alpha = 255;
    text = "";
}
Label::~Label() {
    free();
}
bool Label::loadFromText(const std::string& text) {
    free();
    this->text = text;
    SDL_Color white;
    white.r = 255;
    white.g = 255;
    white.b = 255;
    white.a = 255;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), white);
    if (surf == NULL) return false;
    texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == NULL) {
        SDL_FreeSurface(surf);
        return false;
    }
    dst.w = surf->w;
    dst.h = surf->h;
    SDL_FreeSurface(surf);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, (Uint8)alpha);
    return true;
}
void Label::free() {
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    dst.w = 0;
    dst.h = 0;
    text.clear();
}
void Label::setPosition(int x, int y) {
    posX = (float)x;
    posY = (float)y;
    dst.x = x;
    dst.y = y;
}
void Label::setCenter(int screenW, int screenH) {
    posX = (screenW - dst.w) / 2.0f;
    posY = (screenH - dst.h) / 2.0f;
    dst.x = (int)posX;
    dst.y = (int)posY;
}
void Label::setAlpha(int a) {
    if (a < 0) a = 0;
    if (a > 255) a = 255;
    alpha = a;
    if (texture != NULL) SDL_SetTextureAlphaMod(texture, (Uint8)alpha);
}
int Label::getAlpha() const {
    return alpha;
}
void Label::moveBy(float dx, float dy) {
    posX += dx;
    posY += dy;
    dst.x = (int)(posX + 0.5f);
    dst.y = (int)(posY + 0.5f);
}
void Label::render() {
    if (texture == NULL) return;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}
int Label::getWidth() const {
    return dst.w;
}
int Label::getHeight() const {
    return dst.h;
}
