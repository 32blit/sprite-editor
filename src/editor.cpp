#include "editor.hpp"
#include "control-icons.hpp"

using namespace blit;

Editor::Editor(blit::Point draw_offset, Palette *palette) {
    this->palette = palette;
    this->draw_offset = draw_offset;
    buffer.palette = palette->entries;
    for(auto i = 0u; i < 256; i++) {
        buffer.palette[i] = Pen((uint8_t)i, (uint8_t)i, (uint8_t)i, 255);
    }
}

void Editor::save() {
    buffer.save(current_file);
}

void Editor::load(std::string filename) {

    temp = Surface::load(filename, buf, sizeof(buf));

    if(temp) {
        if(temp->bounds.w > 128 || temp->bounds.h > 128) {
            if(temp->palette) delete[] temp->palette;
            delete temp;
            return;
        }
        current_file = filename;
        if(temp->palette) {
            uint8_t *pen = temp->data;
            for(auto y = 0u; y < buffer.bounds.h; y++) {
                for(auto x = 0u; x < buffer.bounds.w; x++) {
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
            for(auto y = 0u; y < buffer.bounds.h; y++) {
                for(auto x = 0u; x < buffer.bounds.w; x++) {
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
    screen.text(buf, minimal_font, draw_offset - Point(2, 12), false);
}

void Editor::render_help(uint32_t time) {
    constexpr int line_height = 12;

    Point help_offset(draw_offset.x - 2, draw_offset.y + bounds.h + 4);

    screen.pen = Pen(0, 0, 0, 255);
    screen.rectangle(Rect(help_offset, Size(bounds.w + 4, 32)));

    screen.pen = Pen(255, 255, 255, 255);

    control_icon(help_offset, Button::X);
    screen.text("Zoom In", minimal_font, Point(help_offset.x + line_height, help_offset.y));

    control_icon(help_offset + Point(64, 0), Button::Y);
    screen.text("Zoom Out", minimal_font, help_offset + Point(64 + line_height, 0));

    control_icon(help_offset + Point(0, line_height), Button::A);
    control_icon(Point(64 + help_offset.x, help_offset.y + line_height), Button::B);
    switch(mode) {
        case EditMode::Pixel:
            screen.text("Pick", minimal_font, help_offset + Point(line_height, line_height));
            screen.text("Paint", minimal_font, help_offset + Point(line_height + 64, line_height));
            break;
        case EditMode::Sprite:
            screen.text(clipboard ? "Done" : "Copy", minimal_font, help_offset + Point(line_height, line_height));
            screen.text(clipboard ? "Paste" : "Rotate", minimal_font, help_offset + Point(line_height + 64, line_height));
            break;
        case EditMode::Animate:
            screen.text("Start", minimal_font, help_offset + Point(line_height, line_height));
            screen.text("End", minimal_font, help_offset + Point(line_height + 64, line_height));
            break;
    }

}

void Editor::outline_rect(Rect cursor) {
    // avoid outline being 1px too wide/tall
    cursor.w--;
    cursor.h--;
    screen.line(cursor.tl(), cursor.bl());
    screen.line(cursor.tr(), cursor.br());
    // avoid overlap at the corners
    cursor.w-=2;
    cursor.x++;
    screen.line(cursor.tl(), cursor.tr());
    screen.line(cursor.bl(), cursor.br());
}

void Editor::render(uint32_t time, Mouse *mouse) {
    Pen background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    render_status(time);
    render_help(time);

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

    Vec2 viewport_bounds(buffer.bounds.w, buffer.bounds.h);
    viewport_bounds /= view_zoom;
    Rect viewport = Rect(Point(viewport_tl), Size(ceil(viewport_bounds.x), ceil(viewport_bounds.y)));
    viewport = viewport.intersection(Rect(Point(0, 0), Size(buffer.bounds.w - display_offset.x, buffer.bounds.h - display_offset.y)));

    display_offset.x *= view_zoom;
    display_offset.y *= view_zoom;


    Rect dest = Rect(draw_offset + display_offset, Size(viewport.w, viewport.h) * view_zoom);

    screen.stretch_blit(&buffer, viewport, dest);

    if(has_focus) {
        Rect cursor = Rect(draw_offset + ((current_pixel - view_offset) * view_zoom), Size(view_zoom, view_zoom));

        if(mode == EditMode::Pixel) {
            // Selected colour
            screen.pen = palette->fg_pen();
            screen.rectangle(cursor);
        
            // Pulsing inner cursor
            uint8_t pulse = (sinf(time / 250.0f) + 1.0f) * 127;
            screen.pen = Pen(pulse, pulse, pulse, 255);
            screen.pen.a = 255 - pulse;
            cursor.deflate(1);
            screen.rectangle(cursor);
        }

        // sprite cursor
        screen.pen = sprite_cursor_lock ? Pen(255, 64, 64, 128) : Pen(255, 255, 255, 200);
        cursor = Rect(draw_offset + ((current_sprite_offset - view_offset) * view_zoom), sprite_size_pixels * view_zoom);
        outline_rect(cursor);

        // animation start/end cursors
        screen.pen = Pen(128, 255, 128, 200);
        cursor = Rect(draw_offset + (((anim_start * 8) - view_offset) * view_zoom), sprite_size_pixels * view_zoom);
        outline_rect(cursor);

        screen.pen = Pen(128, 128, 255, 200);
        cursor = Rect(draw_offset + (((anim_end * 8) - view_offset) * view_zoom), sprite_size_pixels * view_zoom);
        outline_rect(cursor);
    }

    screen.clip = Rect(Point(0, 0), screen.bounds);

    Point ei = draw_offset - Point(14, 0);
    for(auto i : tool_icons) {
        bool active = (i.sprite == 9 && mode == EditMode::Pixel) || (i.sprite == 10 && mode == EditMode::Sprite) || (i.sprite == 1 && mode == EditMode::Animate);
        if(i.sprite == 11){
            i.help += " " + std::to_string(sprite_size.w);
            i.help += ":" + std::to_string(sprite_size.h);
        }
        ui_icon(&i, ei, mouse, active);
        ei.y += 12;
        screen.sprites->palette[1] = Pen(255, 255, 255, 255);
    }

    screen.pen = background_colour;
}

void Editor::update_sprite_lock() {
    // Handles locking the sprite cursor so it doesn't move around while you detail paint
    if(sprite_size.w == 3 || sprite_size.h == 3) {
        if(view_zoom >= 4) {
            sprite_cursor_lock = true;
            return;
        }
    }
    if(sprite_size.w == 2 || sprite_size.h == 2) {
        if(view_zoom >= 8) {
            sprite_cursor_lock = true;
            return;
        }
    }
    if(sprite_size.w == 1 || sprite_size.h == 1) {
        if(view_zoom >= 16) {
            sprite_cursor_lock = true;
            return;
        }
    }

    sprite_cursor_lock = false;
}

void Editor::update_current_sprite() {
    update_sprite_lock();
    if(sprite_cursor_lock) return;

    current_sprite.x = current_pixel.x / 8 - sprite_size.w / 2;
    current_sprite.y = current_pixel.y / 8 - sprite_size.h / 2;

    if(current_sprite.x < 0) current_sprite.x = 0;
    if(current_sprite.y < 0) current_sprite.y = 0;

    if(current_sprite.x + sprite_size.w >= 16) {
        current_sprite.x = 16 - sprite_size.w;
    }

    if(current_sprite.y + sprite_size.h >= 16) {
        current_sprite.y = 16 - sprite_size.h;
    }

    current_sprite_offset = current_sprite * 8;
}

int Editor::update(uint32_t time, Mouse *mouse) {
    static Point last_cursor(0, 0);
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus) {
        Point ei = draw_offset - Point(14, 0);
        for(auto &i : tool_icons) {
            if(i.sprite == 11){ // sprite size
                if(mouse->dpad.x != 0 || mouse->dpad.y != 0) {
                    if(icon_bounds(ei).contains(mouse->cursor)) {
                        sprite_size.w += mouse->dpad.x;
                        sprite_size.h += mouse->dpad.y;
                        if(sprite_size.w < 1) sprite_size.w = 1;
                        if(sprite_size.h < 1) sprite_size.h = 1;
                        if(sprite_size.w > 3) sprite_size.w = 3;
                        if(sprite_size.h > 3) sprite_size.h = 3;
                        sprite_size_pixels = sprite_size * 8;

                        if(current_sprite.x + sprite_size.w >= 16) {
                            current_sprite.x = 16 - sprite_size.w;
                        }

                        if(current_sprite.y + sprite_size.h >= 16) {
                            current_sprite.y = 16 - sprite_size.h;
                        }

                        current_sprite_offset = current_sprite * 8;

                        if(anim_start.x + sprite_size.w >= 16) {
                            anim_start.x = 16 - sprite_size.w;
                        }

                        if(anim_start.y + sprite_size.h >= 16) {
                            anim_start.y = 16 - sprite_size.h;
                        }

                        if(anim_end.x + sprite_size.w >= 16) {
                            anim_end.x = 16 - sprite_size.w;
                        }

                        if(anim_end.y + sprite_size.h >= 16) {
                            anim_end.y = 16 - sprite_size.h;
                        }


                        return -1;
                    }
                }
            }
            // handle mouse click on icon
            if(mouse->button_a_pressed && icon_bounds(ei).contains(mouse->cursor)) {
                if (i.sprite == 9){
                    mode = EditMode::Pixel;
                    clipboard = false;
                    return -1;
                } else if (i.sprite == 10) {
                    mode = EditMode::Sprite;
                    return -1;
                } else if (i.sprite == 1) {
                    mode = EditMode::Animate;
                    return -1;
                } else {
                    return i.index;
                }
            }
            ei.y += 12;
        }
        return -1;
    }

    Point cursor = mouse->cursor - draw_offset;

    if(view_zoom > 1) view_offset += mouse->dpad;

    if(cursor.x != last_cursor.x || cursor.y != last_cursor.y || mouse->dpad.x != 0 || mouse->dpad.y != 0) {
        current_pixel = (cursor / view_zoom) + Point(view_offset);
        if(current_pixel.x > 127) current_pixel.x = 127;
        if(current_pixel.y > 127) current_pixel.y = 127;
        if(current_pixel.x < 0) current_pixel.x = 0;
        if(current_pixel.y < 0) current_pixel.y = 0;

        update_current_sprite();

        last_cursor = cursor;
    }

    // Handle zooming
    bool new_zoom = false;
    if(mouse->button_y_pressed && view_zoom > 1) {
        view_zoom >>= 1; // Zoom out
        new_zoom = true;
    };
    if(mouse->button_x_pressed && view_zoom < 16) {
        view_zoom <<= 1; // Zoom in
        new_zoom = true;
    };
    if(new_zoom) {
        if(view_zoom == 1) {
            view_offset.x = 0;
            view_offset.y = 0;
            update_sprite_lock();
        }
        else 
        {
            view_offset.x = current_sprite_offset.x;
            view_offset.y = current_sprite_offset.y;
            int visible_pixels = 16 * 8 / view_zoom;
            view_offset.x -= (visible_pixels - sprite_size_pixels.w) / 2;
            view_offset.y -= (visible_pixels - sprite_size_pixels.h) / 2;
            update_sprite_lock();
        }
    }

    if(mode == EditMode::Pixel) {
        if(mouse->button_b) { // Paint
            set_pixel(current_pixel, palette->selected_colour);
        }
        if(mouse->button_a) { // Pick up colour
            palette->selected_colour = get_pixel(current_pixel);
        }
    }
    else if(mode == EditMode::Animate) {
        if(mouse->button_b) { // End
            anim_end = current_sprite;
        }
        if(mouse->button_a) { // Start
            anim_start = current_sprite;
        }
    }
    else if(mode == EditMode::Sprite) {
        if(mouse->button_b_pressed) {
            if(clipboard) {
                for(auto x = 0; x < sprite_size_pixels.w; x++) {
                    for(auto y = 0; y < sprite_size_pixels.h; y++) {
                        set_pixel(current_sprite_offset + Point(x, y), tempdata[x + y * sprite_size_pixels.w]);
                    }
                }
            } else {
                // Rotate 90
                // Since i have to loop to clear to 0 anyway, might as well raw copy from SRC to DST
                for(auto x = 0; x < sprite_size_pixels.w; x++) {
                    for(auto y = 0; y < sprite_size_pixels.h; y++) {
                        tempdata[x + y * sprite_size_pixels.w] = get_pixel(current_sprite_offset + Point(x, y));
                    }
                }

                // actual rotation happens here
                if(sprite_size.w == sprite_size.h) { // Can do 90 degree intervals because our sprite is square
                    for(auto x = 0; x < sprite_size_pixels.w; x++) {
                        for(auto y = 0; y < sprite_size_pixels.h; y++) {
                            set_pixel(current_sprite_offset + Point(sprite_size_pixels.w - 1 - y, x), tempdata[x + y * sprite_size_pixels.w]);
                        }
                    }
                } else { // must do 180
                    for(auto x = 0; x < sprite_size_pixels.w; x++) {
                        for(auto y = 0; y < sprite_size_pixels.h; y++) {
                            set_pixel(current_sprite_offset + Point(sprite_size_pixels.w - 1 - x, sprite_size_pixels.h - 1 - y), tempdata[x + y * sprite_size_pixels.w]);
                        }
                    }
                }

            }
        } else if(mouse->button_a_pressed) {
            if(!clipboard) {
                for(auto x = 0; x < sprite_size_pixels.w; x++) {
                    for(auto y = 0; y < sprite_size_pixels.h; y++) {
                        tempdata[x + y * sprite_size_pixels.w] = get_pixel(current_sprite_offset + Point(x, y));
                    }
                }
            }
            clipboard = !clipboard;
        }
    }

    return -1;
}