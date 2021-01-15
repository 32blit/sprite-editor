#include "main.hpp"
#include "assets.hpp"
#include "src/mouse.hpp"
#include "src/editor.hpp"
#include "src/preview.hpp"
#include "src/dialog.hpp"
#include "src/file-browser.hpp"
#include "engine/api_private.hpp"

using namespace blit;

Screen currentscreen = Screen::Edit;

constexpr int PADDING = 17;

Mouse mouse;
Palette palette(Point(175, PADDING));
Editor editor(Point(PADDING, PADDING), &palette);
Preview preview(Point(PADDING, 240 - 50 - PADDING), &editor);

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

    auto launchPath = blit::get_launch_path();
    if(launchPath) {
        editor.load(launchPath);
    }

    screen.sprites = icons = Surface::load(icon_sprites);
    icons->palette[1] = Pen(255, 255, 255, 255);
}

void render(uint32_t time) {
    screen.sprites = icons;
    screen.pen = Pen(0, 0, 0, 255);
    screen.clear();

    if(currentscreen == Screen::Edit) {
        editor.render(time, &mouse);
        palette.render(time, &mouse);
        preview.render(time, &mouse);
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
        // TODO: fix this eldritch horror, nor... not?
        int editor_action = editor.update(time, &mouse);
        int palette_action = palette.update(time, &mouse);
        int preview_action = preview.update(time, &mouse);

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
            case 3:
                dialog.show("Sure?", "Really clear palette?", [](bool yes){
                    if(yes) {
                        palette.clear();
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
            case 11:
                dialog.show("Tip", "Use the d-pad to configure sprite size", [](bool yes){});
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
