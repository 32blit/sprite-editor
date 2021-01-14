#pragma once
#include "32blit.hpp"
#include "palette.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

enum class EditMode {
  Pixel,
  Sprite
};

class Editor {
    public:
        Editor(blit::Point draw_offset, Palette *palette);
        void render(uint32_t time, Mouse *mouse);
        int update(uint32_t time, Mouse *mouse);
        void set_pixel(blit::Point point, uint8_t colour);
        uint8_t get_pixel(blit::Point point);
        void set_palette(Palette *palette);

        uint8_t data[128 * 128];
        blit::Surface buffer = blit::Surface(data, blit::PixelFormat::P, blit::Size(128, 128));;

        uint8_t tempdata[8 * 8];

        EditMode mode = EditMode::Pixel;

        blit::Point current_pixel = blit::Point(0, 0);
        blit::Point current_sprite = blit::Point(0, 0);

        blit::Point draw_offset;
        Palette *palette;
    
        void reset();
        void save();
        void load(std::string filename);

        std::string current_file;

        bool clipboard = false;

        blit::Surface *temp;
        uint8_t buf[128 * 128 * sizeof(blit::Pen)]; // Buffer for processing input images

        std::vector<UIcon> tool_icons = {
            UIcon{0, "load", 7},
            UIcon{1, "save", 3},
            UIcon{2, "clear", 2},
            UIcon{3, "pixel", 9},
            UIcon{4, "sprite", 10}
        };

    private:
        blit::Size bounds = blit::Size(128, 128);
        bool has_focus = false;
        blit::Vec2 view_offset = blit::Vec2(0, 0);
        int view_zoom = 1;

        void render_help(uint32_t time);
        void render_status(uint32_t time);
        void render_preview(uint32_t time);
};