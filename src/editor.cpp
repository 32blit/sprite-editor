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
    Pen background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    if(has_focus) {
        clip.inflate(2);
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(clip);
        clip.deflate(1);
        screen.pen = background_colour;
        screen.rectangle(clip);
        clip.deflate(1);
    }

    screen.clip = clip;

    Point display_offset(0, 0);
    Vec2 viewport_tl = view_offset;

    if(viewport_tl.x < 0) {
        display_offset.x = fabs(viewport_tl.x);
        viewport_tl.x = 0;
    }
    if(viewport_tl.y < 0) {
        display_offset.y = fabs(viewport_tl.y);
        viewport_tl.y = 0;
    }


    Vec2 viewport_bounds(buffer->bounds.w, buffer->bounds.h);
    viewport_bounds /= view_zoom;
    Rect viewport = Rect(Point(viewport_tl), Size(ceil(viewport_bounds.x), ceil(viewport_bounds.y)));
    viewport = viewport.intersection(Rect(Point(0, 0), Size(buffer->bounds.w - display_offset.x, buffer->bounds.h - display_offset.y)));

    display_offset.x *= view_zoom;
    display_offset.y *= view_zoom;


    Rect dest = Rect(draw_offset + display_offset, Size(viewport.w, viewport.h) * view_zoom);

    screen.stretch_blit(buffer, viewport, dest);

    if(has_focus) {
        Rect cursor = Rect(draw_offset + ((current_pixel - view_offset) * view_zoom), Size(view_zoom, view_zoom));

        // Selected colour
        screen.pen = buffer->palette[selected_colour];
        screen.rectangle(cursor);
    
        // Pulsing inner cursor
        uint8_t pulse = (sinf(time / 250.0f) + 1.0f) * 127;
        screen.pen = Pen(pulse, pulse, pulse, 255);
        screen.pen.a = 255 - pulse;
        cursor.deflate(1);
        screen.rectangle(cursor);
    }

    screen.clip = Rect(Point(0, 0), screen.bounds);
    screen.pen = background_colour;
}

void Editor::update(uint32_t time, Mouse *mouse) {
    static Point last_cursor(0, 0);
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus) return;

    Point cursor = mouse->cursor - draw_offset;

    view_offset += mouse->dpad;

    if(cursor.x != last_cursor.x || cursor.y != last_cursor.y || mouse->dpad.x != 0 || mouse->dpad.y != 0) {
        current_pixel = (cursor / view_zoom) + Point(view_offset);
        if(current_pixel.x > 127) current_pixel.x = 127;
        if(current_pixel.y > 127) current_pixel.y = 127;
        if(current_pixel.x < 0) current_pixel.x = 0;
        if(current_pixel.y < 0) current_pixel.y = 0;

        current_sprite = current_pixel / 8;

        last_cursor = cursor;
    }

    if(mouse->button_y_pressed && view_zoom > 1) {
        view_zoom >>= 1; // Zoom out
        view_offset = Vec2(current_sprite.x * 8, current_sprite.y * 8);
        switch(view_zoom) {
            case 1:
                view_offset = Vec2(0, 0);
                break;
            case 2:
                view_offset -= Vec2(28, 28);
                break;
            case 4:
                view_offset -= Vec2(12, 12);
                break;
            case 8:
                view_offset -= Vec2(4, 4);
                break;
            case 16:
                view_offset -= Vec2(0, 0);
                break;
        }
    };
    if(mouse->button_x_pressed && view_zoom < 16) {
        view_zoom <<= 1; // Zoom in
        view_offset = Vec2(current_sprite.x * 8, current_sprite.y * 8);
        switch(view_zoom) {
            case 1:
                view_offset = Vec2(0, 0);
                break;
            case 2:
                view_offset -= Vec2(28, 28);
                break;
            case 4:
                view_offset -= Vec2(12, 12);
                break;
            case 8:
                view_offset -= Vec2(4, 4);
                break;
            case 16:
                view_offset -= Vec2(0, 0);
                break;
        }
    };

    if(mouse->button_b) {
        set_pixel(current_pixel, selected_colour);
    }
    if(mouse->button_a) {
        selected_colour = get_pixel(current_pixel);
    }
    picked = mouse->button_a;
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
