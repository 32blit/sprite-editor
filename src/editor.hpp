#pragma once
#include "32blit.hpp"
#include "palette.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

enum class EditMode {
  Pixel,
  Sprite,
  Animate
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

        uint8_t tempdata[64 * 64];

        EditMode mode = EditMode::Pixel;

        blit::Size sprite_size = blit::Size(1, 1);

        blit::Point current_pixel = blit::Point(0, 0);
        blit::Point current_sprite = blit::Point(0, 0);
        blit::Point current_sprite_offset = current_sprite * 8;
        blit::Size sprite_size_pixels = sprite_size * 8;

        blit::Point anim_start = blit::Point(0, 0);
        blit::Point anim_end = blit::Point(15, 0);

        blit::Point draw_offset;
        Palette *palette;
    
        void reset();
        bool save();
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
            UIcon{4, "sprite", 10},
            UIcon{5, "animate", 1},
            UIcon{6, "size", 11},
            UIcon{7, "mirror", 6},
            UIcon{8, "roll/rot", 15}
        };

    private:
        blit::Size bounds = blit::Size(128, 128);
        bool has_focus = false;
        blit::Vec2 view_offset = blit::Vec2(0, 0);
        int view_zoom = 1;
        bool sprite_cursor_lock = false;

        void render_help(uint32_t time);
        void render_status(uint32_t time);
        void render_preview(uint32_t time);
        void outline_rect(blit::Rect cursor);
        void update_current_sprite(blit::Vec2 viewport_shift);
        void update_sprite_lock();
        void copy_sprite_to_temp();
};