#include "keyboard.hpp"
#include "control-icons.hpp"
#include "assets.hpp"

using namespace blit;

Keyboard::Keyboard(blit::Point draw_offset) {
    this->draw_offset = draw_offset;
    text.reserve(MAX_INPUT_LENGTH);

    Point keyboard_offset = draw_offset;

    keyboard_offset.x += (bounds.w - (14 * (key_size.w + 2))) / 2;
    keyboard_offset.y += key_size.h + 2 + 4;

    int row = 0;
    int col = 0;
    int k = 0;

    for(auto i = 0; i < normal_letters.length(); i++) {
        auto &key = keys[k];
        if(normal_letters[i] == '\n') {
            key_counts[row] = col;
            col = 0;
            row++;
            continue;
        }
        key.pos.x = col;
        key.pos.y = row;

        key.bounds.x = (key_size.w + 2) * col + offsets[row];
        key.bounds.y = (key_size.h + 2) * row;

        key.bounds.x += keyboard_offset.x;
        key.bounds.y += keyboard_offset.y;

        key.normal = normal_letters[i];
        key.shifted = shifted_letters[i];
    
        key.bounds.h = key_size.h;
        key.bounds.w = key_size.w;
        if(key.normal == " ") {
            key.bounds.w = (key_size.w + 2) * 6 - 2;
            key.normal = "space";
            key.shifted = "space";
            key.function = Function::Space;
        } else if(key.normal ==  "\x08") {
            key.function = Function::BackSpace;
            key.sprite = 5;
        } else if(key.normal == "\x0e") {
            key.function = Function::Shift;
            key.sprite = 3;
        } else if(key.normal == "\x0f") {
            key.function = Function::Shift;
            key.sprite = 3;
        } else if(key.normal == "\x0d") {
            key.bounds.w = (key_size.w + 2) * 2 + 3;
            key.function = Function::Done;
            key.sprite = 1;
            key.normal = "    done";
            key.shifted = "    done";
        } else if (key.shifted == " ") {
            key.shifted = "";
        }
        col++;
        k++;
    }
}

void Keyboard::render(uint32_t time, Mouse *mouse) {
    Pen background_colour = screen.pen;
    Rect clip = Rect(draw_offset, bounds);

    clip.inflate(2);
    screen.pen = has_focus ? Pen(255, 255, 255) : Pen(80, 100, 120);
    outline_rect(clip);
    clip.deflate(2);

    screen.clip = clip;

    screen.pen = Pen(40, 50, 60);
    Rect input_box = clip;
    input_box.h = key_size.h + 4;
    screen.rectangle(input_box);
    input_box.deflate(2);

    screen.pen = Pen(255, 255, 255, 255);
    Size text_size = screen.measure_text(text, font8x8, true);
    screen.text(text, font8x8, input_box, true, TextAlign::center_left);


    uint8_t pulse = (sinf(time / 250.0f) + 1.0f) * 64;
    screen.pen = Pen(255, 255, 255, 64 + pulse);

    screen.rectangle(Rect(input_box.bl() + Point(text_size.w, 0), Size(6, 2)));

    auto temp = screen.sprites->palette[2];
    for(auto k = 0; k < KEY_COUNT; k++) {
        auto &key = keys[k];
        bool current = key.pos.y == current_key.y && (key.pos.y == 4 || key.pos.x == current_key.x);
        screen.sprites->palette[2] = screen.pen = current ? Pen(255, 255, 255) : key.function == Function::Done ? Pen(80, 120, 80) : Pen(80, 100, 120);
        outline_rect(key.bounds);
        if(key.sprite > -1) {
            screen.sprite(key.sprite, (Point(key_size.w, key_size.h) - Point(8, 8)) / 2 + key.bounds.tl());
        }
        screen.text(shift ? key.shifted : key.normal, font8x8, key.bounds, true, TextAlign::center_center);
    }
    screen.sprites->palette[2] = temp;

    screen.clip = Rect(Point(0, 0), screen.bounds);
}

int Keyboard::update(uint32_t time, Mouse *mouse) {
    Rect highlight = Rect(draw_offset, bounds);
    if (highlight.contains(mouse->cursor)) {
        has_focus = true;
    } else {
        has_focus = false;
    }

    if(!has_focus) return -1;

    if(!mouse->cursor_moved) {
        Point old_key = current_key;
        current_key.y += mouse->dpad.y;
        if (current_key.y < 0) current_key.y = 0;
        if (current_key.y > 4) current_key.y = 4;

        // Janky logic to handle the shift and \ keys on the left of the 4th row down
        // TODO: can't actually exit file browse mode without navigating to /
        if(current_key.y == 3 && old_key.y == 2) {
            current_key.x += old_key.x < 11 ? 2 : 1;
        }
        if(current_key.y == 2 && old_key.y == 3) {
            if(old_key.x >= 2) {
                current_key.x -= 2;
            }
            else
            {
                current_key.x = 0;
            }
        }
    
        if(current_key.y != 4) {
            current_key.x += mouse->dpad.x;
            if (current_key.x >= key_counts[current_key.y]) current_key.x = 0;
            if (current_key.x < 0) current_key.x = key_counts[current_key.y] - 1;
        }
    }

    if(mouse->button_y_repeat) backspace();
    if(mouse->button_a_pressed) type(" ");
    if(mouse->button_x_pressed) shift = !shift;

    for(auto k = 0; k < KEY_COUNT; k++) {
        auto &key = keys[k];
        if(mouse->cursor_moved) {
            if(key.bounds.contains(mouse->cursor)) {
                current_key = key.pos;
            }
        }
        if(mouse->button_b_repeat && key.pos.y == current_key.y && (key.pos.y == 4 || key.pos.x == current_key.x)) {
            switch(key.function) {
                case Function::None:
                    type(shift ? key.shifted : key.normal);
                    break;
                case Function::BackSpace:
                    backspace();
                    break;
                case Function::Shift:
                    shift = !shift;
                    break;
                case Function::Space:
                    type(" ");
                    break;
                case Function::Done:
                    if(on_done != nullptr) on_done(text);
                    break;

            }
        }
    }

    return -1;
}

void Keyboard::type(std::string_view letter) {
    if(text.length() < MAX_INPUT_LENGTH) text += letter;
}

void Keyboard::backspace() {
    if(text.length() > 0) text.pop_back();
}

void Keyboard::outline_rect(Rect cursor) {
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

void Keyboard::set_text(std::string text) {
    this->text = text;
}

void Keyboard::set_on_done(void (*func)(std::string)) {
    on_done = func;
}