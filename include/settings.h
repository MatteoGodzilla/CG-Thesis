#pragma once

#define DEFAULT_WIDTH 400
#define DEFAULT_HEIGHT 300

struct Settings {
    int resolution[2] = {DEFAULT_WIDTH,DEFAULT_HEIGHT};
    int viewportSize[2];
    //Flags
    bool shouldResizeBuffers = false;
    bool alwaysDispatch = false;
};
