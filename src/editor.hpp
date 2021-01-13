#pragma once
#include "32blit.hpp"
#include "palette.hpp"
#include "mouse.hpp"

class Editor {
    public:
        Editor(blit::Point draw_offset, Palette *palette);
        void render(uint32_t time);
        void update(uint32_t time, Mouse *mouse);
        void set_pixel(blit::Point point, uint8_t colour);
        uint8_t get_pixel(blit::Point point);
        void set_palette(Palette *palette);
        blit::Surface *buffer = nullptr;
        uint8_t data[128 * 128];

        blit::Point current_pixel = blit::Point(0, 0);
        blit::Point current_sprite = blit::Point(0, 0);

        blit::Point draw_offset;
        Palette *palette;
        void reset();
        void load(std::string filename);

        blit::Surface *temp;
        uint8_t buf[128 * 128 * sizeof(blit::Pen)]; // Buffer for processing input images

    private:
        blit::Size bounds = blit::Size(128, 128);
        bool has_focus = false;
        blit::Vec2 view_offset = blit::Vec2(0, 0);
        int view_zoom = 1;

        void render_help(uint32_t time);
        void render_status(uint32_t time);
        void render_preview(uint32_t time);
};