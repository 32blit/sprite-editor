#include "mouse.hpp"

using namespace blit;

Mouse::Mouse() {
    cursor = Vec2(160, 120);
}

void Mouse::render(uint32_t time) {
    auto pen = screen.pen;
    screen.pen = Pen(255, 255, 255, 255);
    screen.rectangle(Rect(cursor.x - 1, cursor.y - 1, 3, 3));
    screen.pen = Pen(255, 0, 0, 255);
    screen.pixel(cursor);
    screen.pen = pen;
}

void Mouse::update(uint32_t time) {
    dpad_up = buttons & Button::DPAD_UP;
    dpad_down = buttons & Button::DPAD_DOWN;
    dpad_left = buttons & Button::DPAD_LEFT;
    dpad_right = buttons & Button::DPAD_RIGHT;

    dpad_up_pressed = buttons.pressed & Button::DPAD_UP;
    dpad_down_pressed = buttons.pressed & Button::DPAD_DOWN;
    dpad_left_pressed = buttons.pressed & Button::DPAD_LEFT;
    dpad_right_pressed = buttons.pressed & Button::DPAD_RIGHT;

    button_a = buttons & Button::A;
    button_b = buttons & Button::B;
    button_x = buttons & Button::X;
    button_y = buttons & Button::Y;

    button_a_pressed = buttons.pressed & Button::A;
    button_b_pressed = buttons.pressed & Button::B;
    button_x_pressed = buttons.pressed & Button::X;
    button_y_pressed = buttons.pressed & Button::Y;

    dpad = Vec2(0.0f, 0.0f);
    if(buttons & Button::DPAD_UP) dpad.y = -1.0f;
    if(buttons & Button::DPAD_DOWN) dpad.y = 1.0f;
    if(buttons & Button::DPAD_LEFT) dpad.x = -1.0f;
    if(buttons & Button::DPAD_RIGHT) dpad.x = 1.0f;

    cursor += joystick;
}