#include "preview.hpp"
#include "control-icons.hpp"

using namespace blit;

Preview::Preview(blit::Point draw_offset, Editor *editor) {
    this->editor = editor;
    this->draw_offset = draw_offset;
}

void Preview::render_preview(uint32_t time) {
    Pen preview_bg = editor->buffer.palette[editor->palette->selected_background_colour];

    Point preview_pos = draw_offset;

    Rect preview = Rect(preview_pos, Size(bounds.w, bounds.h));
    screen.pen = preview_bg;
    screen.rectangle(preview);

    preview.inflate(2);
    screen.pen = Pen(80, 100, 120);
    outline_rect(preview);

    preview_pos += Point(1, 1);

    if(sprite_size == Size(1, 1)) {
        screen.stretch_blit(&editor->buffer, Rect(current_sprite_offset, Size(8, 8)), Rect(preview_pos, sprite_size_pixels * 4));
        preview_pos.x += sprite_size_pixels.w * 4;
        preview_pos.x += 10;
    }
    
    screen.stretch_blit(&editor->buffer, Rect(current_sprite_offset, sprite_size_pixels), Rect(preview_pos, sprite_size_pixels * 2));
    preview_pos.x += sprite_size_pixels.w * 2;
    preview_pos.x += 10;

    screen.stretch_blit(&editor->buffer, Rect(current_sprite_offset, sprite_size_pixels), Rect(preview_pos, sprite_size_pixels));
    preview_pos.x += sprite_size_pixels.w;
    preview_pos.x += 10;

    Point anim_sprite = anim_start;
    Point anim_range = (anim_end - anim_start);
    anim_range.x /= anim_sprite_size.w;
    anim_range.y /= anim_sprite_size.h;
    anim_range += Point(1, 1);

    if(anim_range.x < 1 || anim_range.y < 1) return;

    int anim_step = paused ? 0 : (time * speed / 200) % (anim_range.x * anim_range.y);

    anim_sprite.x += (anim_step % anim_range.x) * anim_sprite_size.w;
    anim_sprite.y += (anim_step / anim_range.x) * anim_sprite_size.h;

    int anim_scale = 1;
    if(anim_sprite_size == Size(1, 1)) anim_scale = 4;
    if(anim_sprite_size == Size(2, 1) || anim_sprite_size == Size(2, 2) || anim_sprite_size == Size(1, 2)) anim_scale = 2;

    screen.stretch_blit(&editor->buffer, Rect(anim_sprite * 8, anim_sprite_size_pixels), Rect(preview_pos, anim_sprite_size_pixels * anim_scale));
}

void Preview::outline_rect(Rect cursor) {
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

void Preview::render(uint32_t time, Mouse *mouse) {
    Pen background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    clip.inflate(2);
    screen.pen = has_focus ? Pen(255, 255, 255) : Pen(80, 100, 120);
    screen.rectangle(clip);
    clip.deflate(1);
    screen.pen = background_colour;
    screen.rectangle(clip);
    clip.deflate(1);

    screen.clip = clip;

    render_preview(time);

    screen.clip = Rect(Point(0, 0), screen.bounds);

    Point ei = draw_offset - Point(14, 0);
    for(auto i : tool_icons) {
        if(i.sprite == 11 && !locked) continue;
        bool active = (i.sprite == 12 && locked) || (i.sprite == 14 && paused);
        if(i.sprite == 13) { // speeeeeed!
            i.help += " " + std::to_string(speed) + "x"; // I'm going to burn for this
        }
        ui_icon(&i, ei, mouse, active);
        ei.y += 12;
        screen.sprites->palette[1] = Pen(255, 255, 255, 255);
    }

    screen.pen = background_colour;
}

int Preview::update(uint32_t time, Mouse *mouse) {
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!locked) {
        sprite_size = editor->sprite_size;

        current_pixel = editor->current_pixel;
        current_sprite = editor->current_sprite;
        current_sprite_offset = editor->current_sprite_offset;
        sprite_size_pixels = editor->sprite_size_pixels;
    }

    anim_sprite_size = editor->sprite_size;
    anim_sprite_size_pixels = editor->sprite_size_pixels;
    anim_start = editor->anim_start;
    anim_end = editor->anim_end;

    if(!has_focus) {
        Point ei = draw_offset - Point(14, 0);
        for(auto &i : tool_icons) {
            if(i.sprite == 11){
                if(!locked) continue; // only shows up when the preview is locked
                if(mouse->dpad.x != 0 || mouse->dpad.y != 0) {
                    if(icon_bounds(ei).contains(mouse->cursor)) {
                        current_sprite += mouse->dpad;

                        // Don't allow sprite selection outside of sheet!
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
                }
            }
            if(i.sprite == 13){ // sprite size
                if(mouse->dpad.x != 0) {
                    if(icon_bounds(ei).contains(mouse->cursor)) {
                        speed += mouse->dpad.x;
                        if(speed < 1) speed = 1;
                        if(speed > 8) speed = 8;
                        return -1;
                    }
                }
            }

            if(mouse->button_a_pressed && icon_bounds(ei).contains(mouse->cursor)) {
                if (i.sprite == 12){  // Lock
                    locked = !locked;
                    return -1;
                } else if (i.sprite == 14) {  // Play/Pause
                    paused = !paused;
                    return -1;
                } else if (i.sprite == 13) {  // Speed
                    speed += 1;
                    if(speed == 8) speed = 1;
                    return -1;
                } else {
                    return i.index;
                }
            }
            ei.y += 12;
        }
        return -1;
    }

    return -1;
}