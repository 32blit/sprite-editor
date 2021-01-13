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
        unsigned int selected_background_colour = 0;
        blit::Pen bg_pen();
        blit::Pen fg_pen();

        bool picked = false;

    private:
        blit::Size size = blit::Size(7, 7);
        blit::Size bounds;
        blit::Point draw_offset;
        bool has_focus = false;
        blit::Point hover_colour;

        int selected_channel = 0;

        void render_help(uint32_t time);
        void render_status(uint32_t time);

};