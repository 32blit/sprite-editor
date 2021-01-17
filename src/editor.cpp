#include "editor.hpp"
#include "control-icons.hpp"

using namespace blit;

Editor::Editor(blit::Point draw_offset, Palette *palette) {
    this->palette = palette;
    this->draw_offset = draw_offset;
    buffer.palette = palette->entries;
}

bool Editor::save() {
    return buffer.save(current_file) && file_exists(current_file);
}

void Editor::load(std::string filename) {

    temp = Surface::load(filename, buf, sizeof(buf));

    if(temp) {
        reset();

        if(temp->bounds.w > 128 || temp->bounds.h > 128) {
            if(temp->palette) delete[] temp->palette;
            delete temp;
            return;
        }
        current_file = filename;
        if(temp->palette) {
            uint8_t *pen = temp->data;
            for(auto y = 0; y < buffer.bounds.h; y++) {
                for(auto x = 0; x < buffer.bounds.w; x++) {
                    set_pixel(Point(x, y), *pen);
                    pen++;
                }
            }
            for(auto i = 0u; i < 256u; i++) {
                palette->entries[i] = temp->palette[i];
            }
        }
        else{
            Pen *pen = (Pen*)temp->data;
            for(auto y = 0; y < buffer.bounds.h; y++) {
                for(auto x = 0; x < buffer.bounds.w; x++) {
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
    mode = EditMode::Pixel;
    clipboard = false;
    current_pixel = Point(0, 0);
    current_sprite = Point(0, 0);
    current_sprite_offset = current_sprite * 8;
    sprite_size = Size(1, 1);
    sprite_size_pixels = sprite_size * 8;

    anim_start = Point(0, 0);
    anim_end = Point(15, 0);

    view_offset.x = 0;
    view_offset.y = 0;
    view_zoom = 1;
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
        bool disabled = (i.sprite == 15 || i.sprite == 6) && !sprite_cursor_lock; // mirror/roll only enabled when cursor is locked
        bool active = (i.sprite == 9 && mode == EditMode::Pixel) || (i.sprite == 10 && mode == EditMode::Sprite) || (i.sprite == 1 && mode == EditMode::Animate);
        if(i.sprite == 11){
            i.help += " " + std::to_string(sprite_size.w);
            i.help += ":" + std::to_string(sprite_size.h);
        }
        ui_icon(&i, ei, mouse, active, disabled);
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

void Editor::update_current_sprite(Vec2 viewport_shift) {
    update_sprite_lock();
    if(sprite_cursor_lock) {
        // This is basically the inverse of the code that keeps the current sprite centered in the viewport
        // it tries to figure out which sprite you're *looking at* but this includes considering the intent of your cursor
        // plus the intent of your viewport panning.
        // EG: it will only shift horizontally to the next sprite if you *PAN* horizontally and if your cursor has left the previous sprite bounds
        // This only applies while sprite cursor lock is on- IE when you are considered to be "zoomed in" enough to be working on *a* sprite.
        int visible_pixels = 16 * 8 / view_zoom;
        Rect current_sprite_bounds = Rect(current_sprite_offset, sprite_size_pixels);
        current_sprite_bounds.inflate(2); // margin for error for when using d-pad to accurately paint
        if(current_sprite_bounds.contains(current_pixel)) return;
        current_sprite_bounds.deflate(2);

        Point new_sprite_offset = current_sprite_offset;

        if(viewport_shift.x != 0){
            new_sprite_offset.x = view_offset.x;
            new_sprite_offset.x += (visible_pixels - sprite_size_pixels.w) / 2;
        }
    
        if(viewport_shift.y != 0){
            new_sprite_offset.y = view_offset.y;
            new_sprite_offset.y += (visible_pixels - sprite_size_pixels.h) / 2;
        }

        Rect new_sprite_bounds = Rect(new_sprite_offset, sprite_size_pixels);
        // Fudge factor since (1, 1, 1, 1) and (2, 1, 1, 1) are considered to overlap?
        new_sprite_bounds.deflate(1);
        if(new_sprite_bounds.intersects(current_sprite_bounds)) return; // Step the cursor one whole X*Y sprite at a time
        current_sprite_offset = new_sprite_offset;
        current_sprite = current_sprite_offset / 8;
    }
    else
    {
        current_sprite.x = current_pixel.x / 8 - sprite_size.w / 2;
        current_sprite.y = current_pixel.y / 8 - sprite_size.h / 2;
    }

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

void Editor::copy_sprite_to_temp() {
    // Copy into temp buffer
    clipboard = false;
    for(auto x = 0; x < sprite_size_pixels.w; x++) {
        for(auto y = 0; y < sprite_size_pixels.h; y++) {
            tempdata[x + y * 64] = get_pixel(current_sprite_offset + Point(x, y));
        }
    }
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
            if(i.sprite == 6 && sprite_cursor_lock){ // sprite mirror
                if(mouse->dpad.x != 0 || mouse->dpad.y != 0) {
                    if(icon_bounds(ei).contains(mouse->cursor)) {

                        copy_sprite_to_temp();

                        if(mouse->dpad.x != 0){
                            // Mirror LR
                            for(auto x = 0; x < sprite_size_pixels.w; x++) {
                                for(auto y = 0; y < sprite_size_pixels.h; y++) {
                                    set_pixel(current_sprite_offset + Point(sprite_size_pixels.w - 1 - x, y), tempdata[x + y * 64]);
                                }
                            }
                        }
                        if(mouse->dpad.y != 0){
                            // Mirror UD
                            for(auto x = 0; x < sprite_size_pixels.w; x++) {
                                for(auto y = 0; y < sprite_size_pixels.h; y++) {
                                    set_pixel(current_sprite_offset + Point(x, sprite_size_pixels.h - 1 - y), tempdata[x + y * 64]);
                                }
                            }
                        }
                    }
                }
            }
            if(i.sprite == 15 && sprite_cursor_lock){ // sprite roll
                if(mouse->dpad.x != 0 || mouse->dpad.y != 0) {
                    if(icon_bounds(ei).contains(mouse->cursor)) {
                        copy_sprite_to_temp();
                        for(auto x = 0; x < sprite_size_pixels.w; x++) {
                            for(auto y = 0; y < sprite_size_pixels.h; y++) {
                                int nx = x - mouse->dpad.x;
                                int ny = y - mouse->dpad.y;
                                if(nx < 0) nx = sprite_size_pixels.w - 1;
                                if(ny < 0) ny = sprite_size_pixels.h - 1;
                                if(nx == sprite_size_pixels.w) nx = 0;
                                if(ny == sprite_size_pixels.h) ny = 0;
                                set_pixel(current_sprite_offset + Point(x, y), tempdata[nx + ny * 64]);
                            }
                        }
                    }
                }
            }
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

    if(mouse->cursor_moved || mouse->dpad.x != 0 || mouse->dpad.y != 0) {
        current_pixel = (cursor / view_zoom) + Point(view_offset);
        if(current_pixel.x > 127) current_pixel.x = 127;
        if(current_pixel.y > 127) current_pixel.y = 127;
        if(current_pixel.x < 0) current_pixel.x = 0;
        if(current_pixel.y < 0) current_pixel.y = 0;

        update_current_sprite(mouse->dpad);
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
                        set_pixel(current_sprite_offset + Point(x, y), tempdata[x + y * 64]);
                    }
                }
            } else {
                // Rotate 90
                // Since i have to loop to clear to 0 anyway, might as well raw copy from SRC to DST
                copy_sprite_to_temp();

                // actual rotation happens here
                if(sprite_size.w == sprite_size.h) { // Can do 90 degree intervals because our sprite is square
                    for(auto x = 0; x < sprite_size_pixels.w; x++) {
                        for(auto y = 0; y < sprite_size_pixels.h; y++) {
                            set_pixel(current_sprite_offset + Point(sprite_size_pixels.w - 1 - y, x), tempdata[x + y * 64]);
                        }
                    }
                } else { // must do 180
                    for(auto x = 0; x < sprite_size_pixels.w; x++) {
                        for(auto y = 0; y < sprite_size_pixels.h; y++) {
                            set_pixel(current_sprite_offset + Point(sprite_size_pixels.w - 1 - x, sprite_size_pixels.h - 1 - y), tempdata[x + y * 64]);
                        }
                    }
                }

            }
        } else if(mouse->button_a_pressed) {
            if(!clipboard) {
                copy_sprite_to_temp();
            }
            clipboard = !clipboard;
        }
    }

    return -1;
}