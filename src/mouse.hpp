#pragma once
#include "32blit.hpp"


class Mouse {
    public:
        Mouse();
        void render(uint32_t time);
        void update(uint32_t time);
        blit::Point cursorPosition();
        blit::Vec2 cursor;
        blit::Vec2 dpad;
        
        bool dpad_up, dpad_down, dpad_left, dpad_right, 
        button_a, button_b, button_x, button_y, 
        button_a_pressed, button_b_pressed, button_x_pressed, button_y_pressed,
        button_menu;

    private:
};