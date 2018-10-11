#ifndef GENERALS_DISPLAY_H_
#define GENERALS_DISPLAY_H_

#include <ncurses.h>

#include "generals.h"

void InitGeneralsColor();
void Print(const GeneralsGame&, int = -1);

#endif
