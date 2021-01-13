#include "main.hpp"
#include "assets.hpp"
#include "src/mouse.hpp"
#include "src/editor.hpp"
#include "src/dialog.hpp"
#include "src/file-browser.hpp"

using namespace blit;

Screen currentscreen = Screen::Edit;

struct UIcon {
    unsigned int index;
    std::string help;
    unsigned int sprite;
};

Mouse mouse;
Palette palette(Point(175, 14 + 17));
Editor editor(Point(17, 14 + 17), &palette);

Dialog dialog;
FileBrowser file_browser;

Surface *icons;

// TODO menu-like class to encapsulate icon lists
Point editor_icons_position(4, 14 + 17);
std::vector<UIcon> editor_icons = {
    UIcon{0, "load", 7},
    UIcon{1, "save", 3},
    UIcon{2, "clear", 2}
};

void palette_load(std::string filename) {
    palette.load(filename);
    currentscreen = Screen::Edit;
}

void editor_load(std::string filename) {
    editor.load(filename);
    currentscreen = Screen::Edit;
}

Rect icon_bounds(Point position) {
    return Rect(position, Size(10, 10));
}

void draw_icon(int index, Point position) {
    Rect box = icon_bounds(position);
    Pen temp = screen.sprites->palette[2];
    screen.pen = box.contains(mouse.cursor) ? Pen(40, 40, 40, 255) : Pen(10, 10, 10, 255);
    screen.sprites->palette[2] = box.contains(mouse.cursor) ? Pen(255, 255, 255, 255) : temp;
    screen.rectangle(box);
    box.deflate(1);
    screen.sprite(index, box.tl());
    screen.sprites->palette[2] = temp;
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
        editor.render(time);
        palette.render(time);

        Point ei = editor.draw_offset;
        ei.x -= 14;

        for(auto &i : editor_icons) {
            draw_icon(i.sprite, ei);
            ei.y += 12;
        }

        ei = palette.draw_offset;
        ei.x -= 14;

        for(auto &i : editor_icons) {
            draw_icon(i.sprite, ei);
            ei.y += 12;
        }

        dialog.draw(&mouse);
        mouse.render(time);
    }

    if(currentscreen == Screen::LoadSprites) {
        file_browser.render();
    }
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
        if(mouse.button_a_pressed) {
            Point ei = editor.draw_offset;
            ei.x -= 14;
            for(auto &i : editor_icons) {
                if(icon_bounds(ei).contains(mouse.cursor)) {
                    switch(i.index) {
                        case 0:
                            currentscreen = Screen::LoadSprites;
                            file_browser.set_extensions({".bmp", ".ssr", ".ssp"});
                            file_browser.set_on_file_open(editor_load);
                            file_browser.init();
                            break;
                        case 1:
                            currentscreen = Screen::SaveSprites;
                            break;
                        case 2:
                            dialog.show("Sure?", "Really clear sprites?", [](bool yes){
                                if(yes) {
                                    editor.reset();
                                }
                            });
                            break;
                    }
                }
                ei.y += 12;
            }

            ei = palette.draw_offset;
            ei.x -= 14;
            for(auto &i : editor_icons) {
                if(icon_bounds(ei).contains(mouse.cursor)) {
                    switch(i.index) {
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
                    }
                }
                ei.y += 12;
            }
        }
        editor.update(time, &mouse);
        palette.update(time, &mouse);
    } else {
        /*if(mouse.button_b_pressed) {
            currentscreen = Screen::Edit;
        }*/
    }
}
