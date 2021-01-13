#include "editor.hpp"

using namespace blit;

Editor::Editor(blit::Point draw_offset, Palette *palette) {
    this->palette = palette;
    this->draw_offset = draw_offset;
    buffer = new Surface(data, blit::PixelFormat::P, blit::Size(128, 128));
    buffer->palette = new Pen[256];
    for(auto i = 0u; i < 256; i++) {
        buffer->palette[i] = Pen((uint8_t)i, (uint8_t)i, (uint8_t)i, 255);
    }
    palette->entries = buffer->palette;
}

void Editor::load(std::string filename) {

    temp = Surface::load(filename, buf, sizeof(buf));

    if(temp) {
        if(temp->bounds.w > 128 || temp->bounds.h > 128) {
            if(temp->palette) delete[] temp->palette;
            delete temp;
            return;
        }
        if(temp->palette) {
            uint8_t *pen = temp->data;
            for(auto y = 0u; y < buffer->bounds.h; y++) {
                for(auto x = 0u; x < buffer->bounds.w; x++) {
                    set_pixel(Point(x, y), *pen);
                    pen++;
                }
            }
            for(auto i = 0u; i < 256; i++) {
                palette->entries[i] = temp->palette[i];
            }
        }
        else{
            Pen *pen = (Pen*)temp->data;
            for(auto y = 0u; y < buffer->bounds.h; y++) {
                for(auto x = 0u; x < buffer->bounds.w; x++) {
                    auto index = palette->add(*pen);
                    pen++;
                    set_pixel(Point(x, y), index);
                }
            }
        }

        if(temp->palette) delete[] temp->palette;
        delete temp;
    }
}

void Editor::reset() {
    for(auto i = 0u; i < 128 * 128; i++){
        data[i] = 0;
    }
}

void Editor::set_pixel(Point point, uint8_t colour) {
    data[point.x + point.y * 128] = colour;
}

uint8_t Editor::get_pixel(Point point) {
    return data[point.x + point.y * 128];
}

void Editor::render_status(uint32_t time) {
    screen.pen = Pen(255, 255, 255, 255);

    char buf[100] = "";
    snprintf(buf, 100, "spr:%02i:%02i  pix:%03i:%03i", current_sprite.x, current_sprite.y, current_pixel.x, current_pixel.y);
    screen.text(buf, minimal_font, Point(15, 14 + 5), false);
}

void Editor::render_help(uint32_t time) {
    constexpr int line_height = 12;

    int help_labels_y = draw_offset.y + bounds.h + 5;
    int help_labels_x = draw_offset.x - 2;

    screen.pen = Pen(255, 255, 255, 255);

    screen.sprites->palette[1] = Pen(99, 175, 227, 255); // Blue
    screen.sprite(0, Point(help_labels_x, help_labels_y));
    screen.text("Zoom In", minimal_font, Point(help_labels_x + line_height, help_labels_y));

    screen.sprites->palette[1] = Pen(100, 246, 178, 255); // Green
    screen.sprite(0, Point(64 + help_labels_x, help_labels_y), SpriteTransform::R270);
    screen.text("Zoom Out", minimal_font, Point(64 + help_labels_x + line_height, help_labels_y));

    screen.sprites->palette[1] = Pen(236, 92, 181, 255); // Pink/Red
    screen.sprite(0, Point(help_labels_x, help_labels_y + line_height), SpriteTransform::R90);
    screen.text("Pick", minimal_font, Point(help_labels_x + line_height, help_labels_y + line_height));

    screen.sprites->palette[1] = Pen(234, 226, 81, 255); // Yellow
    screen.sprite(0, Point(64 + help_labels_x, help_labels_y + line_height), SpriteTransform::R180);
    screen.text("Paint", minimal_font, Point(64 + help_labels_x + line_height, help_labels_y + line_height));

    screen.sprites->palette[1] = Pen(80, 100, 120, 255);
}

void Editor::render_preview(uint32_t time) {
    Pen preview_bg = buffer->palette[palette->selected_background_colour];

    constexpr int padding = 17;

    screen.pen = preview_bg;
    screen.pen.a = 255;

    Rect preview = Rect(draw_offset.x, 240 - 32 - padding, bounds.w, 32);
    preview.inflate(2);
    screen.rectangle(preview);

    screen.stretch_blit(buffer, Rect(current_sprite * 8, Size(8, 8)), Rect(padding, 240 - 32 - padding, 32, 32));
    screen.stretch_blit(buffer, Rect(current_sprite * 8, Size(8, 8)), Rect(padding + 32 + 10, 240 - 32 - padding, 16, 16));
    screen.stretch_blit(buffer, Rect(current_sprite * 8, Size(8, 8)), Rect(padding + 32 + 10 + 16 + 10, 240 - 32 - padding, 8, 8));

    Point anim_sprite = current_sprite;
    anim_sprite.x = (time / 200) & 0x0f;
    screen.stretch_blit(buffer, Rect(anim_sprite * 8, Size(8, 8)), Rect(padding + 32 + 10 + 16 + 10 + 8 + 10, 240 - 32 - padding, 32, 32));
}

void Editor::render(uint32_t time) {
    Pen background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    render_status(time);
    render_help(time);
    render_preview(time);

    clip.inflate(2);
    screen.pen = has_focus ? Pen(255, 255, 255) : Pen(80, 100, 120);
    screen.rectangle(clip);
    clip.deflate(1);
    screen.pen = background_colour;
    screen.rectangle(clip);
    clip.deflate(1);

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
        screen.pen = palette->fg_pen();
        screen.rectangle(cursor);
    
        // Pulsing inner cursor
        uint8_t pulse = (sinf(time / 250.0f) + 1.0f) * 127;
        screen.pen = Pen(pulse, pulse, pulse, 255);
        screen.pen.a = 255 - pulse;
        cursor.deflate(1);
        screen.rectangle(cursor);

        if(view_zoom < 16) {
            screen.pen = Pen(128, 128, 128, 255);
            screen.pen.a = 255;
            cursor = Rect(draw_offset + (((current_sprite * 8) - view_offset) * view_zoom), Size(8 * view_zoom, 8 * view_zoom));
            screen.line(cursor.tl(), cursor.tr());
            screen.line(cursor.bl(), cursor.br());
            screen.line(cursor.tl(), cursor.bl());
            screen.line(cursor.tr(), cursor.br());
        }
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
        set_pixel(current_pixel, palette->selected_colour);
    }
    if(mouse->button_a) {
        palette->selected_colour = get_pixel(current_pixel);
    }
}