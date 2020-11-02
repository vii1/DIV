#include "time.h"

clock_t deltaTime = 0;

static clock_t lastClock = 0;

void time_init()
{
    deltaTime = 0;
    lastClock = clock();
}

void time_frame()
{
    clock_t currentClock = clock();
    deltaTime = currentClock - lastClock;
    lastClock = currentClock;
}
