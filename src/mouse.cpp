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
    dpad_up = buttons.state & Button::DPAD_UP;
    dpad_down = buttons.state & Button::DPAD_DOWN;
    dpad_left = buttons.state & Button::DPAD_LEFT;
    dpad_right = buttons.state & Button::DPAD_RIGHT;

    dpad_up_pressed = buttons.pressed & Button::DPAD_UP;
    dpad_down_pressed = buttons.pressed & Button::DPAD_DOWN;
    dpad_left_pressed = buttons.pressed & Button::DPAD_LEFT;
    dpad_right_pressed = buttons.pressed & Button::DPAD_RIGHT;

    button_a = buttons.state & Button::A;
    button_b = buttons.state & Button::B;
    button_x = buttons.state & Button::X;
    button_y = buttons.state & Button::Y;
    button_menu = buttons.state & Button::MENU;

    button_a_pressed = buttons.pressed & Button::A;
    button_b_pressed = buttons.pressed & Button::B;
    button_x_pressed = buttons.pressed & Button::X;
    button_y_pressed = buttons.pressed & Button::Y;
    button_menu_pressed = buttons.pressed & Button::MENU;

    button_a_repeat = ar_button_a.next(time, buttons & Button::A);
    button_b_repeat = ar_button_b.next(time, buttons & Button::B);
    button_x_repeat = ar_button_x.next(time, buttons & Button::X);
    button_y_repeat = ar_button_y.next(time, buttons & Button::Y);

    dpad = Vec2(0.0f, 0.0f);
    if(ar_dpad_u.next(time, buttons & Button::DPAD_UP)) dpad.y = -1.0f;
    if(ar_dpad_d.next(time, buttons & Button::DPAD_DOWN)) dpad.y = 1.0f;
    if(ar_dpad_l.next(time, buttons & Button::DPAD_LEFT)) dpad.x = -1.0f;
    if(ar_dpad_r.next(time, buttons & Button::DPAD_RIGHT)) dpad.x = 1.0f;

    cursor += joystick;
    if(cursor.x < 0) cursor.x = 0;
    if(cursor.x >= screen.bounds.w) cursor.x = screen.bounds.w - 1;
    if(cursor.y < 0) cursor.y = 0;
    if(cursor.y >= screen.bounds.h) cursor.y = screen.bounds.h - 1;

    cursor_moved = cursor.x != last_cursor.x || cursor.y != last_cursor.y;
    last_cursor = cursor;
}