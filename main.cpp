#include "main.hpp"
#include "assets.hpp"
#include "src/mouse.hpp"
#include "src/editor.hpp"
#include "src/dialog.hpp"
#include "src/file-browser.hpp"
#include "engine/api_private.hpp"

using namespace blit;

Screen currentscreen = Screen::Edit;

Mouse mouse;
Palette palette(Point(175, 14 + 17));
Editor editor(Point(17, 14 + 17), &palette);

Dialog dialog;
FileBrowser file_browser;

Surface *icons;

std::string load_filename;

void palette_load(std::string filename) {
    palette.load(filename);
    currentscreen = Screen::Edit;
}

void editor_load(std::string filename) {
    load_filename = filename;
    dialog.show("Load file?", filename, [](bool yes){
        if(yes) {
            editor.load(load_filename);
            currentscreen = Screen::Edit;
        }
    });
}

void init() {
    set_screen_mode(ScreenMode::hires);

    editor.load("s4m_ur4i-dingbads.bmp");

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
        editor.render(time, &mouse);
        palette.render(time, &mouse);
    }

    if(currentscreen == Screen::LoadSprites) {
        file_browser.render();
    }

    dialog.draw(&mouse);
    mouse.render(time);
}

void update(uint32_t time) {
    mouse.update(time);
    if(dialog.update(&mouse)) return;

    if(mouse.button_menu_pressed) {
        switch(currentscreen) {
            case Screen::Edit:
                currentscreen = Screen::Menu;
                break;
            case Screen::Menu:
                currentscreen = Screen::Edit;
                break;
            default:
                break;
        }
    }

    if(currentscreen == Screen::LoadSprites) {
        file_browser.update(time);
        return;
    }

    if(currentscreen == Screen::Edit) {
        int editor_action = editor.update(time, &mouse);
        int palette_action = palette.update(time, &mouse);

        switch(palette_action) {
            case 0:
                currentscreen = Screen::LoadPalette;
                file_browser.set_extensions({".act", ".pal"});
                file_browser.set_on_file_open(palette_load);
                file_browser.init();
                break;
            case 1:
                currentscreen = Screen::SavePalette;
                break;
            case 2:
                dialog.show("Sure?", "Really reset palette?", [](bool yes){
                    if(yes) {
                        palette.reset();
                    }
                });
                break;
            default:
                break;
        }

        switch(editor_action) {
            case 0:
                currentscreen = Screen::LoadSprites;
                file_browser.set_extensions({".bmp", ".spriterw", ".spritepk"});
                file_browser.set_on_file_open(editor_load);
                file_browser.init();
                break;
            case 1:
                dialog.show("Sure?", "Save to " + editor.current_file, [](bool yes){
                    if(yes) {
                        editor.save();
                    }
                });
                break;
            case 2:
                dialog.show("Sure?", "Really clear sprites?", [](bool yes){
                    if(yes) {
                        editor.reset();
                    }
                });
                break;
            default:
                break;
        }
    } else {
        /*if(mouse.button_b_pressed) {
            currentscreen = Screen::Edit;
        }*/
    }
}
