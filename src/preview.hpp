#pragma once
#include "32blit.hpp"
#include "palette.hpp"
#include "editor.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

class Preview {
    public:
        Preview(blit::Point draw_offset, Editor *editor);
        void render(uint32_t time, Mouse *mouse);
        int update(uint32_t time, Mouse *mouse);

        std::vector<UIcon> tool_icons = {
            UIcon{0, "lock", 12},
            UIcon{1, "play/pause", 14},
            UIcon{2, "speed", 13},
            UIcon{3, "nudge", 11},
        };

    private:
        blit::Size bounds = blit::Size(128, 50);
        Editor *editor;
        blit::Point draw_offset;
        bool has_focus = false;
        bool locked = false;
        bool paused = false;
        int speed = 1;

        // Values copied from the editor to support locking
        blit::Size sprite_size = blit::Size(1, 1);

        blit::Point current_pixel = blit::Point(0, 0);
        blit::Point current_sprite = blit::Point(0, 0);
        blit::Point current_sprite_offset = current_sprite * 8;
        blit::Size sprite_size_pixels = sprite_size * 8;
        // end le values

        blit::Size anim_sprite_size = blit::Size(1, 1);
        blit::Size anim_sprite_size_pixels = anim_sprite_size * 8;
        blit::Point anim_start = blit::Point(0, 0);
        blit::Point anim_end = blit::Point(15, 0);

        void outline_rect(blit::Rect cursor);
        void render_preview(uint32_t time);
};