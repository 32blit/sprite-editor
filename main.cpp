#include "main.hpp"
#include "assets.hpp"
#include "src/mouse.hpp"
#include "src/editor.hpp"

using namespace blit;

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
}

void render(uint32_t time) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, 0, 320, 14));
    screen.pen = Pen(0, 0, 0);
    screen.text("32blit Sprite Editor", minimal_font, Point(5, 4));

    editor.render(time);
    palette.render(time);

    constexpr int line_height = 12;
    constexpr int padding = 17;

    screen.sprites = icons;

    screen.sprite(0, Point(padding, 128 + padding + 14 + padding));
    screen.text("Zoom In", minimal_font, Point(padding + line_height, 128 + padding + 14 + padding));

    screen.sprite(0, Point(64 + padding, 128 + padding + 14 + padding), SpriteTransform::R270);
    screen.text("Zoom Out", minimal_font, Point(64 + padding + line_height, 128 + padding + 14 + padding));

    screen.sprite(0, Point(padding, 128 + padding + 14 + padding + line_height), SpriteTransform::R90);
    screen.text("Paint", minimal_font, Point(padding + line_height, 128 + padding + 14 + padding + line_height));

    screen.sprite(0, Point(64 + padding, 128 + padding + 14 + padding + line_height), SpriteTransform::R180);
    screen.text("Pick", minimal_font, Point(64 + padding + line_height, 128 + padding + 14 + padding + line_height));

    screen.stretch_blit(editor.buffer, Rect(editor.current_sprite * 8, Size(8, 8)), Rect(175, 128 + padding + 14 + padding, 32, 32));
    screen.stretch_blit(editor.buffer, Rect(editor.current_sprite * 8, Size(8, 8)), Rect(175 + 32 + 10, 128 + padding + 14 + padding, 16, 16));
    screen.stretch_blit(editor.buffer, Rect(editor.current_sprite * 8, Size(8, 8)), Rect(175 + 32 + 10 + 16 + 10, 128 + padding + 14 + padding, 8, 8));

    mouse.render(time);
}

void update(uint32_t time) {
    mouse.update(time);
    editor.update(time, &mouse);
    palette.update(time, &mouse);

    if(palette.picked) editor.selected_colour = palette.selected_colour;
    if(editor.picked) palette.selected_colour = editor.selected_colour;
}
