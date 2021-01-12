#include "palette.hpp"

using namespace blit;

Palette::Palette(blit::Point draw_offset, Pen *palette) {
    this->draw_offset = draw_offset;
    bounds = Size(16 * (size.w + 1), 16 * (size.h + 1));
    bounds.w--;
    bounds.h--;
    entries = palette;
}

void Palette::render(uint32_t time) {
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

    auto pen = screen.pen;
    for(auto i = 0u; i < 256; i++) {
        screen.pen = entries[i];
        Point pixel(i & 0xf, i >> 4);
        pixel.x *= size.w + 1;
        pixel.y *= size.h + 1;
        screen.rectangle(Rect(pixel + draw_offset, size));
    }
    screen.pen = pen;
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