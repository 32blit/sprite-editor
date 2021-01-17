#pragma once
#include "32blit.hpp"
#include "mouse.hpp"

const blit::Pen BTN_GREEN = blit::Pen(100, 246, 178, 255);
const blit::Pen BTN_BLUE = blit::Pen(99, 175, 227, 255);
const blit::Pen BTN_PINK = blit::Pen(236, 92, 181, 255);
const blit::Pen BTN_YELLOW = blit::Pen(234, 226, 81, 255);
const blit::Pen WHITE = blit::Pen(255, 255, 255, 255);

struct UIcon {
    unsigned int index;
    std::string help;
    unsigned int sprite;
};

void ui_icon(UIcon *i, blit::Point position, Mouse *mouse, bool active=false, bool disabled=false);
blit::Rect icon_bounds(blit::Point position);
void control_icon(blit::Point location, blit::Button button);