#include "palette.hpp"
#include "control-icons.hpp"

using namespace blit;

Palette::Palette(blit::Point draw_offset) {
    this->draw_offset = draw_offset;
    bounds = Size(16 * (size.w + 1), 16 * (size.h + 1));
    bounds.w--;
    bounds.h--;
}

void Palette::load(std::string filename) {
    
}

void Palette::reset() {
    for(auto i = 0u; i < 256; i++) {
        entries[i] = Pen((uint8_t)i, (uint8_t)i, (uint8_t)i, 255);
    }
}

Pen Palette::fg_pen() {
    return entries[selected_colour];
}

Pen Palette::bg_pen() {
    return entries[selected_background_colour];
}


void Palette::render_status(uint32_t time) {
    constexpr int padding = 17;
    screen.pen = Pen(255, 255, 255, 255);

    char buf[100] = "";

    snprintf(buf, 100, "FG #%02x%02x%02x BG #%02x%02x%02x", fg_pen().r, fg_pen().g, fg_pen().b, bg_pen().r, bg_pen().g, bg_pen().b);
    screen.text(buf, minimal_font, Point(draw_offset.x - 2, 14 + 5), false);
}

void Palette::render_help(uint32_t time) {
    constexpr int line_height = 12;

    Point help_offset(draw_offset.x - 2, draw_offset.y + bounds.h + 5);

    screen.pen = Pen(0, 0, 0, 255);
    screen.rectangle(Rect(help_offset, Size(bounds.w + 4, 32)));

    screen.pen = Pen(255, 255, 255, 255);

    control_icon(help_offset, Button::X);
    screen.text("Invert", minimal_font, Point(help_offset.x + line_height, help_offset.y));

    control_icon(help_offset + Point(64, 0), Button::Y);
    screen.text("Replace", minimal_font, help_offset + Point(64 + line_height, 0));

    control_icon(help_offset + Point(0, line_height), Button::A);
    screen.text("Pick", minimal_font, help_offset + Point(line_height, line_height));

    control_icon(Point(64 + help_offset.x, help_offset.y + line_height), Button::B);
    screen.text("Pick BG", minimal_font, help_offset + Point(line_height + 64, line_height));
}

void Palette::render(uint32_t time, Mouse *mouse) {
    auto background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    render_status(time);
    render_help(time);

    clip.inflate(2);
    screen.pen = has_focus ? Pen(255, 255, 255) : Pen(80, 100, 120);
    screen.rectangle(clip);
    clip.deflate(1);
    screen.pen = Pen(0, 0, 0);
    screen.rectangle(clip);
    clip.deflate(1);

    Rect selected = Rect((selected_colour & 0xf) * (size.w + 1), (selected_colour >> 4) * (size.h + 1), size.w + 2, size.h + 2);
    selected.x += draw_offset.x - 1;
    selected.y += draw_offset.y - 1;
    screen.pen = Pen(255, 255, 255, 255);
    screen.rectangle(selected);

    if(has_focus) {
        uint8_t pulse = (sinf(time / 250.0f) + 1.0f) * 64;
        Rect hover = Rect(hover_colour.x * (size.w + 1), hover_colour.y * (size.h + 1), size.w + 2, size.h + 2);
        hover.x += draw_offset.x - 1;
        hover.y += draw_offset.y - 1;
        screen.pen = Pen(255, 255, 255, 127 + pulse);
        screen.rectangle(hover);
    }

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
            screen.pen = Pen(0, 0, 0, 255);
            screen.rectangle(bar_rect);

            bar_rect.deflate(2);
            bar_rect.w = width;
            bar_rect.x--;

            screen.pen = bar_colour;
            screen.rectangle(bar_rect);
        }
    }

    Point ei = draw_offset - Point(14, 0);
    for(auto &i : tool_icons) {
        ui_icon(&i, ei, mouse);
        ei.y += 12;
    }

    screen.pen = background_colour;
}

int Palette::update(uint32_t time, Mouse *mouse) {
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus){
        if(mouse->button_a_pressed) {
            Point ei = draw_offset - Point(14, 0);
            for(auto &i : tool_icons) {
                if(icon_bounds(ei).contains(mouse->cursor)) {
                    return i.index;
                }
                ei.y += 12;
            }
        }
        return -1;
    }

    Point cursor = mouse->cursor - draw_offset;
    hover_colour = cursor;
    hover_colour.x /= (size.w + 1);
    hover_colour.y /= (size.h + 1);

    if(mouse->button_b_pressed) {
        selected_background_colour = hover_colour.x + hover_colour.y * 16;
    }

    selected_channel += mouse->dpad.y;
    if(selected_channel > 3) selected_channel = 0;
    if(selected_channel < 0) selected_channel = 3;

    if(mouse->button_x_pressed) {
        Pen current = entries[selected_colour];
        entries[selected_colour] = Pen(~current.r, ~current.g, ~current.b, current.a);
    }

    switch(selected_channel){
        case 0: // red
            entries[selected_colour].r += mouse->dpad.x * 8;
            entries[selected_colour].r = (int)(entries[selected_colour].r / 8) * 8;
            break;
        case 1: // green
            entries[selected_colour].g += mouse->dpad.x * 4;
            entries[selected_colour].g = (int)(entries[selected_colour].g / 4) * 4;
            break;
        case 2: // blue
            entries[selected_colour].b += mouse->dpad.x * 8;
            entries[selected_colour].b = (int)(entries[selected_colour].b / 8) * 8;
            break;
        case 3: // alpha
            entries[selected_colour].a += mouse->dpad.x * 8;
            entries[selected_colour].a = (int)(entries[selected_colour].a / 8) * 8;
            break;
    }

    if(mouse->button_y_pressed) {
        entries[hover_colour.x + hover_colour.y * 16] = entries[selected_colour];
    }
    if(mouse->button_a_pressed) {
        selected_colour = hover_colour.x + hover_colour.y * 16;
    }
    return -1;
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