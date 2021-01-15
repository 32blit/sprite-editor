#pragma once
#include "32blit.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

class Palette {
    public:
        Palette(blit::Point draw_offset);
        void render(uint32_t time, Mouse *mouse);
        int update(uint32_t time, Mouse *mouse);
        int add(blit::Pen pen);
        blit::Pen entries[256];
        unsigned int palette_entries = 0;
        unsigned int selected_colour = 0;
        unsigned int selected_background_colour = 0;
        blit::Pen bg_pen();
        blit::Pen fg_pen();

        blit::Point draw_offset;
        void reset();
        void load(std::string filename);

        std::vector<UIcon> tool_icons = {
            UIcon{0, "load", 7},
            UIcon{1, "save", 3},
            UIcon{2, "clear", 2}
        };

    private:
        blit::Size size = blit::Size(7, 7);
        blit::Size bounds;
        bool has_focus = false;
        blit::Point hover_colour;

        int selected_channel = 0;

        void render_help(uint32_t time);
        void render_status(uint32_t time);
        void outline_rect(blit::Rect cursor);

};