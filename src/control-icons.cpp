#include "control-icons.hpp"

using namespace blit;


void control_icon(Point location, Button button) {
    auto temp = screen.sprites->palette[1];
    switch(button){
        case Button::Y:
            screen.sprites->palette[1] = BTN_GREEN; // Green
            screen.sprite(0, location, SpriteTransform::R270);
            break;
        case Button::X:
            screen.sprites->palette[1] = BTN_BLUE; // Blue
            screen.sprite(0, location);
            break;
        case Button::A:
            screen.sprites->palette[1] = BTN_PINK; // Pink/Red
            screen.sprite(0, location, SpriteTransform::R90);
            break;
        case Button::B:
            screen.sprites->palette[1] = BTN_YELLOW; // Yellow
            screen.sprite(0, location, SpriteTransform::R180);
            break;
        default:
            break;
    }
    screen.sprites->palette[1] = temp;
}

Rect icon_bounds(Point position) {
    return Rect(position, Size(10, 10));
}

void ui_icon(UIcon *i, Point position, Mouse *mouse, bool active) {
    Rect box = icon_bounds(position);
    bool hover = box.contains(mouse->cursor);
    Pen temp = screen.sprites->palette[2];
    screen.pen = hover ? Pen(40, 40, 40, 255) : Pen(10, 10, 10, 255);
    screen.sprites->palette[2] = box.contains(mouse->cursor) ? (active ? Pen(255, 255, 255, 255) : Pen(255, 128, 128, 255)): (active ? Pen(255, 0, 0, 255) : temp);
    screen.rectangle(box);
    box.deflate(1);
    screen.sprite(i->sprite, box.tl());
    screen.sprites->palette[2] = temp;
    if(hover) {
        Size textsize = screen.measure_text(i->help, minimal_font);
        textsize.h = box.h;
        Rect label_bg = Rect(box.tr(), textsize);
        label_bg.inflate(1);
        label_bg.x += 2;
        label_bg.w += 1;
        screen.rectangle(label_bg);
        screen.pen = active ? Pen(255, 128, 128, 255) : Pen(255, 255, 255, 255);
        screen.text(i->help, minimal_font, box.tr() + Point(3, 1));
    }
}