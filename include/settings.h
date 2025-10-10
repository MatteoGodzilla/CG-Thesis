#pragma once
#include "viewportFilter.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

//TODO: move this entirely under ui.h
struct Settings {
    int resolution[2] = {DEFAULT_WIDTH,DEFAULT_HEIGHT};
    int viewportSize[2];
    //Flags
    bool shouldResizeBuffers = false;
    bool alwaysDispatch = false;
    ViewportFilter filter = VF_FILL;
};
