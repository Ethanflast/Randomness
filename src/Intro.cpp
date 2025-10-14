#include "Intro.h"
Intro::Intro(SDL_Renderer* renderer, TTF_Font* font, const std::string& firstText, const std::string& secondText, int screenW, int screenH, float fadeDuration, float holdDuration) {
    this->renderer = renderer;
    this->font = font;
    this->firstText = firstText;
    this->secondText = secondText;
    this->screenW = screenW;
    this->screenH = screenH;
    firstLabel = NULL;
    secondLabel = NULL;
    this->fadeDuration = fadeDuration;
    this->holdDuration = holdDuration;
    timer = 0.0f;
    state = FIRST_FADE_IN;
}
Intro::~Intro() {
    if (firstLabel != NULL) {
        delete firstLabel;
        firstLabel = NULL;
    }
    if (secondLabel != NULL) {
        delete secondLabel;
        secondLabel = NULL;
    }
}
void Intro::start() {
    if (firstLabel != NULL) {
        delete firstLabel;
        firstLabel = NULL;
    }
    if (secondLabel != NULL) {
        delete secondLabel;
        secondLabel = NULL;
    }
    firstLabel = new Label(renderer, font);
    secondLabel = new Label(renderer, font);
    firstLabel->loadFromText(firstText);
    secondLabel->loadFromText(secondText);
    firstLabel->setCenter(screenW, screenH);
    secondLabel->setCenter(screenW, screenH);
    firstLabel->setAlpha(0);
    secondLabel->setAlpha(0);
    state = FIRST_FADE_IN;
    timer = 0.0f;
}
void Intro::update(float dt) {
    if (state == DONE) return;
    timer += dt;
    if (state == FIRST_FADE_IN) {
        float progress = timer / fadeDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        int a = (int)(progress * 255.0f);
        firstLabel->setAlpha(a);
        if (timer >= fadeDuration) {
            firstLabel->setAlpha(255);
            state = FIRST_HOLD;
            timer = 0.0f;
        }
    } else if (state == FIRST_HOLD) {
        if (timer >= holdDuration) {
            state = FIRST_FADE_OUT;
            timer = 0.0f;
        }
    } else if (state == FIRST_FADE_OUT) {
        float progress = timer / fadeDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        int a = (int)((1.0f - progress) * 255.0f);
        firstLabel->setAlpha(a);
        if (timer >= fadeDuration) {
            firstLabel->setAlpha(0);
            state = SECOND_FADE_IN;
            timer = 0.0f;
            secondLabel->setCenter(screenW, screenH);
        }
    } else if (state == SECOND_FADE_IN) {
        float progress = timer / fadeDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        int a = (int)(progress * 255.0f);
        secondLabel->setAlpha(a);
        if (timer >= fadeDuration) {
            secondLabel->setAlpha(255);
            state = SECOND_HOLD;
            timer = 0.0f;
        }
    } else if (state == SECOND_HOLD) {
        if (timer >= holdDuration) {
            state = DONE;
        }
    }
}
void Intro::render() {
    if (firstLabel != NULL && (state == FIRST_FADE_IN || state == FIRST_HOLD || state == FIRST_FADE_OUT)) firstLabel->render();
    if (secondLabel != NULL && (state == SECOND_FADE_IN || state == SECOND_HOLD || state == DONE)) secondLabel->render();
}
bool Intro::isDone() const {
    return state == DONE;
}
