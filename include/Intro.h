#ifndef INTRO_H
#define INTRO_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "Label.h"
class Intro {
public:
    Intro(SDL_Renderer* renderer, TTF_Font* font, const std::string& firstText, const std::string& secondText, int screenW, int screenH, float fadeDuration = 1.0f, float holdDuration = 2.5f);
    ~Intro();
    void start();
    void update(float dt);
    void render();
    bool isDone() const;
private:
    enum State { FIRST_FADE_IN, FIRST_HOLD, FIRST_FADE_OUT, SECOND_FADE_IN, SECOND_HOLD, DONE };
    State state;
    Label* firstLabel;
    Label* secondLabel;
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::string firstText;
    std::string secondText;
    int screenW;
    int screenH;
    float fadeDuration;
    float holdDuration;
    float timer;
};
#endif
