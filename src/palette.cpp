#include "palette.hpp"

using namespace blit;

Palette::Palette(blit::Point draw_offset, Pen *palette) {
    this->draw_offset = draw_offset;
    bounds = Size(16 * (size.w + 1), 16 * (size.h + 1));
    bounds.w--;
    bounds.h--;
    entries = palette;
}

Pen Palette::pen() {
    return entries[selected_colour];
}

void Palette::render(uint32_t time) {
    auto background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    if(has_focus) {
        clip.inflate(2);
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(clip);
        clip.deflate(1);
        screen.pen = Pen(0, 0, 0);
        screen.rectangle(clip);
        clip.deflate(1);
    }

    Rect selected = Rect((selected_colour & 0xf) * (size.w + 1), (selected_colour >> 4) * (size.h + 1), size.w + 2, size.h + 2);
    selected.x += draw_offset.x - 1;
    selected.y += draw_offset.y - 1;
    screen.pen = Pen(255, 255, 255, 255);
    screen.rectangle(selected);

    Rect hover = Rect(hover_colour.x * (size.w + 1), hover_colour.y * (size.h + 1), size.w + 2, size.h + 2);
    hover.x += draw_offset.x - 1;
    hover.y += draw_offset.y - 1;
    screen.pen = Pen(128, 128, 128, 200);
    screen.rectangle(hover);

    for(auto i = 0u; i < 256; i++) {
        screen.pen = entries[i];
        Point pixel(i & 0xf, i >> 4);
        pixel.x *= size.w + 1;
        pixel.y *= size.h + 1;
        screen.rectangle(Rect(pixel + draw_offset, size));
    }

    if(has_focus) {
        Point colour_picker = Point(draw_offset.x - 12, 240 - 32 - 17);
        for(auto i = 0u; i < 4; i++ ) {
            Point bar = colour_picker + Point(0, 8 * i);
            screen.pen = i == selected_channel ? Pen(255, 255, 255, 255) : Pen(128, 128, 128, 255);
            int width = 0;
            Pen bar_colour = Pen(0, 0, 0, 0);
            switch(i) {
                case 0: // r
                    screen.text("R", minimal_font, bar);
                    width = entries[selected_colour].r >> 1;
                    bar_colour = Pen(255, 0, 0, 255);
                    break;
                case 1: // g
                    screen.text("G", minimal_font, bar);
                    width = entries[selected_colour].g >> 1;
                    bar_colour = Pen(0, 255, 0, 255);
                    break;
                case 2: // b
                    screen.text("B", minimal_font, bar);
                    width = entries[selected_colour].b >> 1;;
                    bar_colour = Pen(0, 0, 255, 255);
                    break;
                case 3: // a
                    screen.text("A", minimal_font, bar);
                    width = entries[selected_colour].a >> 1;
                    bar_colour = Pen(255, 255, 255, 255);
                    break;
            }
            bar.x += 11;
            Rect bar_rect = Rect(bar, Size(130, 8));
            if(i == selected_channel) {
                screen.rectangle(bar_rect);
            }
            bar_rect.deflate(1);
            screen.pen = background_colour;
            screen.rectangle(bar_rect);

            bar_rect.deflate(2);
            bar_rect.w = width;
            bar_rect.x--;

            screen.pen = bar_colour;
            screen.rectangle(bar_rect);
        }
    }

    screen.pen = background_colour;
}

void Palette::update(uint32_t time, Mouse *mouse) {
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus) return;

    Point cursor = mouse->cursor - draw_offset;
    hover_colour = cursor;
    hover_colour.x /= (size.w + 1);
    hover_colour.y /= (size.h + 1);

    if(mouse->button_y_pressed) {
        background_colour = entries[hover_colour.x + hover_colour.y * 16];
        background_colour.a = 255;
    }

    if(mouse->dpad_down_pressed) {
        selected_channel++;
        if(selected_channel > 3) selected_channel = 0;
    }

    if(mouse->dpad_up_pressed) {
        selected_channel--;
        if(selected_channel < 0) selected_channel = 3;
    }

    if(mouse->button_x_pressed) {
        Pen current = entries[selected_colour];
        entries[selected_colour] = Pen(~current.r, ~current.g, ~current.b, current.a);
    }

    switch(selected_channel){
        case 0: // red
            if(mouse->dpad_right_pressed) entries[selected_colour].r += 8;
            if(mouse->dpad_left_pressed) entries[selected_colour].r -= 8;
            entries[selected_colour].r = (int)(entries[selected_colour].r / 8) * 8;
            break;
        case 1: // green
            if(mouse->dpad_right_pressed) entries[selected_colour].g += 4;
            if(mouse->dpad_left_pressed) entries[selected_colour].g -= 4;
            entries[selected_colour].g = (int)(entries[selected_colour].g / 4) * 4;
            break;
        case 2: // blue
            if(mouse->dpad_right_pressed) entries[selected_colour].b += 8;
            if(mouse->dpad_left_pressed) entries[selected_colour].b -= 8;
            entries[selected_colour].b = (int)(entries[selected_colour].b / 8) * 8;
            break;
        case 3: // alpha
            if(mouse->dpad_right_pressed) entries[selected_colour].a += 8;
            if(mouse->dpad_left_pressed) entries[selected_colour].a -= 8;
            entries[selected_colour].a = (int)(entries[selected_colour].a / 8) * 8;
            break;
    }

    if(mouse->button_b_pressed) {
        entries[hover_colour.x + hover_colour.y * 16] = entries[selected_colour];
    }
    if(mouse->button_a_pressed) {
        selected_colour = hover_colour.x + hover_colour.y * 16;
    }
    picked = mouse->button_a_pressed;
}

int Palette::add(Pen pen) {
    for(auto i = 0u; i < palette_entries; i++) {
        if(entries[i] == pen) {
            return i;
        }
    }
    if(palette_entries < 256) {
        entries[palette_entries] = pen;
        palette_entries++;
        return palette_entries - 1;
    }
}