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
    if (firstLabel) { delete firstLabel; firstLabel = NULL; }
    if (secondLabel) { delete secondLabel; secondLabel = NULL; }
}

void Intro::start() {
    if (firstLabel) { delete firstLabel; firstLabel = NULL; }
    if (secondLabel) { delete secondLabel; secondLabel = NULL; }
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
        firstLabel->setAlpha((int)(progress * 255.0f));
        if (timer >= fadeDuration) { firstLabel->setAlpha(255); state = FIRST_HOLD; timer = 0.0f; }
    } else if (state == FIRST_HOLD) {
        if (timer >= holdDuration) { state = FIRST_FADE_OUT; timer = 0.0f; }
    } else if (state == FIRST_FADE_OUT) {
        float progress = timer / fadeDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        firstLabel->setAlpha((int)((1.0f - progress) * 255.0f));
        if (timer >= fadeDuration) { firstLabel->setAlpha(0); state = SECOND_FADE_IN; timer = 0.0f; secondLabel->setCenter(screenW, screenH); }
    } else if (state == SECOND_FADE_IN) {
        float progress = timer / fadeDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        secondLabel->setAlpha((int)(progress * 255.0f));
        if (timer >= fadeDuration) { secondLabel->setAlpha(255); state = SECOND_HOLD; timer = 0.0f; }
    } else if (state == SECOND_HOLD) {
        if (timer >= holdDuration) { state = DONE; }
    }
}

void Intro::render() {
    if (firstLabel && (state == FIRST_FADE_IN || state == FIRST_HOLD || state == FIRST_FADE_OUT)) firstLabel->render();
    if (secondLabel && (state == SECOND_FADE_IN || state == SECOND_HOLD || state == DONE)) secondLabel->render();
}

bool Intro::isDone() const { return state == DONE; }

void Intro::run(Window& window, FPSOverlay& fpsOverlay) {
    const int SCREEN_FPS = 60;
    const int SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;
    Uint32 last = SDL_GetTicks();
    SDL_Event event;
    bool running = true;
    float fpsTimer = 0.0f;
    int framesCounted = 0;

    while (running && !isDone()) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) { if (event.type == SDL_QUIT) running = false; }
        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;

        update(dt);
        SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 0, 255);
        window.clear();
        render();

        framesCounted++;
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) {
            fpsOverlay.update(framesCounted / fpsTimer);
            fpsTimer = 0.0f;
            framesCounted = 0;
        }

        fpsOverlay.render();
        window.display();

        Uint32 frameTicks = SDL_GetTicks() - frameStart;
        if (frameTicks < SCREEN_TICK_PER_FRAME) SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
    }
}
