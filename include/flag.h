#pragma once

class Flag{
public:
    Flag(bool initial = false);
    void set();
    void clear();
    bool getState();

private:
    bool state;
};
