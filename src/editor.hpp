#pragma once
#include "32blit.hpp"
#include "palette.hpp"
#include "mouse.hpp"

class Editor {
    public:
        Editor(blit::Point draw_offset);
        void render(uint32_t time);
        void update(uint32_t time, Mouse *mouse);
        void set_pixel(blit::Point point, uint8_t colour);
        uint8_t get_pixel(blit::Point point);
        void set(blit::Surface *src, Palette *palette);
        blit::Surface *buffer = nullptr;
        uint8_t data[128 * 128];
        unsigned int selected_colour = 0;

        blit::Point current_pixel = blit::Point(0, 0);
        blit::Point current_sprite = blit::Point(0, 0);

        bool picked = false;

    private:
        blit::Size bounds = blit::Size(128, 128);
        blit::Point draw_offset;
        bool has_focus = false;
        blit::Vec2 view_offset = blit::Vec2(0, 0);
        int view_zoom = 1;

};