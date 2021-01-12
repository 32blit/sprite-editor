#pragma once
#include "32blit.hpp"
#include "mouse.hpp"

class Palette {
    public:
        Palette(blit::Point draw_offset, blit::Pen *palette);
        void render(uint32_t time);
        void update(uint32_t time, Mouse *mouse);
        int add(blit::Pen pen);
        blit::Pen *entries;
        unsigned int palette_entries = 0;
        unsigned int selected_colour = 0;
        blit::Pen background_colour = blit::Pen(0, 0, 0, 255);
        blit::Pen pen();

        bool picked = false;

    private:
        blit::Size size = blit::Size(7, 7);
        blit::Size bounds;
        blit::Point draw_offset;
        bool has_focus = false;
        blit::Point hover_colour;

        int selected_channel = 0;

};