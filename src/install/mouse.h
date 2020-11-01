#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "main.h"

void read_mouse();

extern int prevMouseX, prevMouseY;
extern int mouseX, mouseY;
extern word mouseButtons;

#endif // __MOUSE_H__
