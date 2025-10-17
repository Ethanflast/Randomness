#ifndef RUNSCENE_H
#define RUNSCENE_H

#include "Window.h"
#include "Intro.h"
#include "Video.h"
#include "FPSOverlay.h"

class RunScene {
public:
    RunScene(Window* window, FPSOverlay* fpsOverlay);
    void runIntro(Intro* intro);
    void runVideo(Video* video, int x, int y, int w, int h);
private:
    Window* window;
    FPSOverlay* fpsOverlay;
};

#endif
