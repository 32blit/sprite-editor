#include "editor.hpp"

using namespace blit;

Editor::Editor(blit::Point draw_offset) {
    this->draw_offset = draw_offset;
    buffer = new Surface(data, blit::PixelFormat::P, blit::Size(128, 128));
    buffer->palette = new Pen[256];
    for(auto i = 0u; i < 256; i++) {
        buffer->palette[i] = Pen((uint8_t)i, (uint8_t)i, (uint8_t)i, 255);
    }
}

void Editor::set_pixel(Point point, uint8_t colour) {
    data[point.x + point.y * 128] = colour;
}

uint8_t Editor::get_pixel(Point point) {
    return data[point.x + point.y * 128];
}

void Editor::render(uint32_t time) {
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

    screen.clip = clip;

    Rect viewport = Rect(Point(view_offset), buffer->bounds / view_zoom);
    Rect dest = Rect(draw_offset, Size(bounds.w, bounds.h));
    screen.stretch_blit(buffer, viewport, dest);

    screen.pen = Pen(255, 255, 255, 255);
    screen.rectangle(Rect(draw_offset + ((current_pixel - view_offset) * view_zoom), Size(view_zoom, view_zoom)));

    screen.clip = Rect(Point(0, 0), screen.bounds);
}

void Editor::update(uint32_t time, Mouse *mouse) {
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus) return;

    Point cursor = mouse->cursor - draw_offset;

    current_pixel = (cursor / view_zoom) + Point(view_offset);
    current_sprite = current_pixel / 8;

    if(mouse->button_y_pressed && view_zoom > 1) view_zoom >>= 1;
    if(mouse->button_x_pressed && view_zoom < 16) view_zoom <<= 1;

    view_offset += mouse->dpad / view_zoom;
    if(view_offset.x < 0) view_offset.x = 0;
    if(view_offset.y < 0) view_offset.y = 0;

    if(mouse->button_a) {
        set_pixel(current_pixel, selected_colour);
    }
    if(mouse->button_b) {
        selected_colour = get_pixel(current_pixel);
    }
    picked = mouse->button_b;
}

void Editor::set(blit::Surface *src, Palette *palette) {
    if(!src->palette) {
        Pen *pen = (Pen*)src->data;
        for(auto y = 0u; y < src->bounds.h; y++) {
            for(auto x = 0u; x < src->bounds.w; x++) {
                auto index = palette->add(*pen);
                pen++;
                set_pixel(Point(x, y), index);
            }
        }
    }
}
