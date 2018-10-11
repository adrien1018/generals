#include "generals_display.h"

template <class T>
void Print(const GeneralsGame& g, T&& mp) {
  auto& stat = g.GetStat();
  for (size_t i = 0, h = 0; h < g.Height(); h++) {
    for (size_t w = 0; w < g.Width(); w++, i++) {
      char buf[4] = "   ";
      if (mp[i].troops) {
        int q = std::max(0, (int)mp[i].troops);
        if (q < 1000) snprintf(buf, 4, "%3d", q);
        else if (q < 10000) snprintf(buf, 4, "%2dH", q / 100);
        else if (q < 100000) snprintf(buf, 4, "%2dK", q / 1000);
        else if (q < 1000000) snprintf(buf, 4, "%2dW", q / 10000);
        else snprintf(buf, 4, "%2dM", std::min(99, q / 1000000));
      } else if (mp[i].type == GeneralsCell::kMountain ||
                 mp[i].type == GeneralsCell::kUnknownMountain) {
        buf[1] = '#';
      }
      int own = mp[i].owner;
      int cent = own == -1 ? 9 : own + 1, side = cent;
      switch (mp[i].type) {
        case GeneralsCell::kSpace: break;
        case GeneralsCell::kCity: cent = 11; own == -1 ? side = 11 : 0; break;
        case GeneralsCell::kGeneral: cent = 12; break;
        case GeneralsCell::kGeneralCity: cent = 10; break;
        case GeneralsCell::kMountain: cent = side = 10; break;
        case GeneralsCell::kUnknown: // fall through
        case GeneralsCell::kUnknownMountain: cent = side = 12; break;
      }
      move(h, w * 3);
      attron(COLOR_PAIR(side)); addch(buf[0]); attroff(COLOR_PAIR(side));
      attron(COLOR_PAIR(cent)); addch(buf[1]); attroff(COLOR_PAIR(cent));
      attron(COLOR_PAIR(side)); addch(buf[2]); attroff(COLOR_PAIR(side));
    }
  }
  attron(COLOR_PAIR(11));
  mvprintw(0, g.Width() * 3 + 3, "Round:%7d", (int)g.Round());
  attroff(COLOR_PAIR(11));
  for (size_t i = 0; i < stat.size(); i++) {
    int out = !stat[i].alive;
    attron(COLOR_PAIR(i + 1 + out * 16));
    mvprintw(i + 1, g.Width() * 3 + 3, "%8d", stat[i].troops);
    mvprintw(i + 1, g.Width() * 3 + 12, "%4d", stat[i].lands);
    attroff(COLOR_PAIR(i + 1 + out * 16));
    if (out) {
      attron(COLOR_PAIR(12));
      mvaddch(i + 1, g.Width() * 3 + 1, ' ');
      mvaddch(i + 1, g.Width() * 3 + 2, ' ');
      attroff(COLOR_PAIR(12));
    }
  }
  refresh();
}

void InitGeneralsColor() {
  start_color();
  init_color(16, 1000, 0, 0);
  init_color(17, 0, 500, 0);
  init_color(18, 0, 0, 1000);
  init_color(19, 500, 0, 0);
  init_color(20, 0, 500, 500);
  init_color(21, 400, 600, 0);
  init_color(22, 500, 0, 500);
  init_color(23, 1000, 500, 0);
  init_color(24, 750, 750, 750); // space
  init_color(25, 250, 250, 250); // obstacle
  init_color(26, 500, 500, 500); // city
  init_color(27, 0, 0, 0); // middle: generals
  init_color(28, 1000, 1000, 1000);
  for (int i = 1; i <= 13; i++) {
    init_pair(i, 28, i + 15);
    init_pair(i + 16, 27, i + 15);
  }
}

void Print(const GeneralsGame& g, int player) {
  if (player < 0) {
    Print(g, g.GetMap());
  } else {
    Print(g, g.GetMap(player));
  }
}
