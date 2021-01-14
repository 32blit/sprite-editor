#pragma once
#include "32blit.hpp"
#include "mouse.hpp"

struct UIcon {
    unsigned int index;
    std::string help;
    unsigned int sprite;
};

void ui_icon(UIcon *i, blit::Point position, Mouse *mouse, bool active=false);
blit::Rect icon_bounds(blit::Point position);
void control_icon(blit::Point location, blit::Button button);