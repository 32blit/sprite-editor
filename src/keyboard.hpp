#pragma once
#include "32blit.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

#define KEY_COUNT 53
#define MAX_INPUT_LENGTH 256

class Keyboard {
    public:
        Keyboard(blit::Point draw_offset);
        void render(uint32_t time, Mouse *mouse);
        int update(uint32_t time, Mouse *mouse);

        std::string text = "";
        void type(std::string_view letter);
        void backspace();
        void set_on_done(void (*func)(std::string));
        void set_text(std::string text);

    private:
        enum Function {
            None,
            Space,
            Shift,
            BackSpace,
            Done
        };
        struct Key {
            blit::Rect bounds;
            std::string normal = " ";
            std::string shifted = " ";
            bool hover = false;
            int sprite = -1;
            Function function = Function::None;
            blit::Point pos = blit::Point(0, 0);
        };

        void (*on_done)(std::string) = nullptr;
        blit::Size bounds = blit::Size(320 - 17 - 17, 99);
        blit::Point draw_offset;
        bool has_focus = false;
        bool shift = false;
        blit::Size key_size = blit::Size(16, 14);
        blit::Point current_key = blit::Point(0, 0);

        const std::string normal_letters = "`1234567890-=\x08\nqwertyuiop[]\nasdfghjkl;'#\n\x0e\\zxcvbnm,./\x0f\x0d\n ";
        const std::string shifted_letters = "`!\" $% &*()_+\x08\nQWERTYUIOP{}\nASDFGHJKL:@~\n\x0e|ZXCVBNM<>?\x0f\x0d\n ";
        const int8_t offsets[5] = {0, 8, 18, -10, 70};
        uint8_t key_counts[5];
        Key keys[KEY_COUNT];
        void outline_rect(blit::Rect cursor);
};