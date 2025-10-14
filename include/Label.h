#ifndef LABEL_H
#define LABEL_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
class Label {
public:
    Label(SDL_Renderer* renderer, TTF_Font* font);
    ~Label();
    bool loadFromText(const std::string& text);
    void free();
    void setPosition(int x, int y);
    void setCenter(int screenW, int screenH);
    void setAlpha(int a);
    int getAlpha() const;
    void moveBy(float dx, float dy);
    void render();
    int getWidth() const;
    int getHeight() const;
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* texture;
    SDL_Rect dst;
    float posX;
    float posY;
    int alpha;
    std::string text;
};
#endif
