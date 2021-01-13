#include "main.hpp"
#include "assets.hpp"
#include "src/mouse.hpp"
#include "src/editor.hpp"

using namespace blit;

Screen currentscreen = Screen::Edit;

Mouse mouse;
Editor editor(Point(17, 14 + 17));
Palette palette(Point(175, 14 + 17), editor.buffer->palette);

Surface *icons;
Surface *temp;
uint8_t buf[128 * 128 * sizeof(Pen)]; // Buffer for processing input images

void init() {
    set_screen_mode(ScreenMode::hires);

    temp = Surface::load("s4m_ur4i-dingbads.bmp", buf, sizeof(buf));
    
    if(temp) {
        editor.set(temp, &palette);

        if(temp->palette) delete[] temp->palette;
        delete temp;
    }

    screen.sprites = icons = Surface::load(icon_sprites);
    icons->palette[1] = Pen(255, 255, 255, 255);
}

void render(uint32_t time) {
    screen.sprites = icons;
    screen.pen = Pen(0, 0, 0, 0);
    screen.pen.a = 255;
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, 0, 320, 14));
    screen.pen = Pen(0, 0, 0); 
    switch(currentscreen) {
        case Screen::Edit:
            screen.text("32blit Sprite Editor", minimal_font, Point(5, 4));
            break;
        case Screen::Menu:
            screen.text("Menu", minimal_font, Point(5, 4));
            break;
        case Screen::SavePalette:
            screen.text("Save Palette", minimal_font, Point(5, 4));
            break;
        case Screen::SaveSprites:
            screen.text("Save Sprites", minimal_font, Point(5, 4));
            break;
        case Screen::LoadPalette:
            screen.text("Load Palette", minimal_font, Point(5, 4));
            break;
        case Screen::LoadSprites:
            screen.text("Load Sprites", minimal_font, Point(5, 4));
            break;
    }

    if(currentscreen == Screen::Edit) {
        editor.render(time);
        palette.render(time);
        mouse.render(time);
    }
}

void update(uint32_t time) {
    mouse.update(time);

    if(mouse.button_menu_pressed) {
        switch(currentscreen) {
            case Screen::Edit:
                currentscreen = Screen::Menu;
                break;
            case Screen::Menu:
                currentscreen = Screen::Edit;
                break;
        }
    }

    if(currentscreen == Screen::Edit) {
        editor.update(time, &mouse);
        palette.update(time, &mouse);

        if(palette.picked) {
            editor.selected_colour = palette.selected_colour;
            editor.selected_background_colour = palette.selected_background_colour;
        }
        if(editor.picked) palette.selected_colour = editor.selected_colour;
    }
}
