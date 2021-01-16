#include "file-browser.hpp"

#include "engine/engine.hpp"
#include "control-icons.hpp"

using namespace blit;

FileBrowser::FileBrowser(const Font &font) : Menu("", nullptr, 0, font) {
    item_h = font.char_h + 6;
    item_adjust_y = 0;

    header_h = item_h;
    footer_h = 0;
    margin_y = 0;

    header_background = Pen(0, 0, 0);
    header_foreground = Pen(255, 255, 255);
    background_colour = Pen(0, 0, 0);
    foreground_colour = Pen(255, 255, 255);
    selected_item_background = Pen(40, 50, 60);

    display_rect.w = 320;
    display_rect.h = 240;
    display_rect.x = 0;
    display_rect.y = 0;
}

void FileBrowser::set_height(uint32_t height) {
    display_rect.h = height;
}

void FileBrowser::init() {
    update_list();
}

bool FileBrowser::update(uint32_t time, Mouse *mouse) {
    has_focus = display_rect.contains(mouse->cursor);
    if(!has_focus) return false;


    if(mouse->dpad.y < 0) {
        do {
        if(--current_item < 0) {
            current_item += num_items;
        }
        } while(items[current_item].label == nullptr );
    } else if(mouse->dpad.y > 0) {
        do {
        if(++current_item == num_items) {
            current_item = 0;
        }
        } while(items[current_item].label == nullptr );
    }

    if (mouse->cursor_moved) {
        int x = display_rect.x;
        int y = display_rect.y;
        int w = display_rect.w;

        // y region to clip to
        y += header_h;
        int display_height = display_rect.h - (header_h + footer_h);

        y += (int)scroll_offset + margin_y;

        // items
        for(int i = 0; i < num_items; i++) {
            auto &item = items[i];

            if ( item.label != nullptr ) {
                Rect item_rect(display_rect.x, y, display_rect.w, item_h);
                if(item_rect.contains(mouse->cursor)) current_item = i;
            }

            y += item_h + item_spacing;
        }
    }

    if (mouse->button_a_pressed) {
        item_activated(items[current_item]);
    }

    // scrolling
    int total_height = num_items * (item_h + item_spacing);
    int display_height = display_rect.h - (header_h + footer_h + margin_y * 2);

    int current_y = current_item * (item_h + item_spacing);
    int target_scroll = display_height / 2 - current_y;

    // clamp
    target_scroll = std::min(0, std::max(-(total_height - display_height), target_scroll));

    scroll_offset += (target_scroll - scroll_offset) * 0.2f;

    update_menu(time);

    if(mouse->button_b_pressed) {
        if(cur_dir == "/") {
            return true;
        }
        else {
            // go up
            auto pos = cur_dir.find_last_of('/', cur_dir.length() - 2);
            if(pos == std::string::npos)
                cur_dir = "";
            else
                cur_dir = cur_dir.substr(0, pos + 1);

            update_list();
        }
    }

    return false;
}

void FileBrowser::render()
{
    Menu::render();

    const int iconSize = font.char_h > 8 ? 12 : 8;

    const int32_t backTextWidth = screen.measure_text("Back", font).w;

    Rect r(display_rect.tl(), Size(display_rect.w, header_h));

    screen.pen = header_foreground;

    r.x += item_padding_x;
    r.w -= item_padding_x * 2;

    // back icon
    if(!cur_dir.empty() && has_focus) {
        Point iconOffset(-(backTextWidth + iconSize + 2), (item_h - iconSize) / 2); // from the top-right

        screen.text("Back", font, r, true, TextAlign::center_right);
        control_icon(r.tr() + iconOffset, Button::B);
    }
}

void FileBrowser::set_extensions(std::set<std::string> exts) {
    file_exts = exts;
}

void FileBrowser::set_on_file_open(void (*func)(std::string)) {
    on_file_open = func;
}

void FileBrowser::set_current_dir(const std::string &dir) {
    if(dir[0] != '/')
        cur_dir = "/" + dir;
    else
        cur_dir = dir;

    if(cur_dir.back() != '/')
        cur_dir += "/";

    update_list();
}

void FileBrowser::update_list() {
    title = cur_dir;

    files = list_files(cur_dir.substr(0, cur_dir.length() - 1));

    std::sort(files.begin(), files.end(), [](FileInfo &a, FileInfo & b){return a.name < b.name;});

    if(file_exts.empty())
        return;

    // filter by extensions
    files.erase(std::remove_if(files.begin(), files.end(), [this](const FileInfo &f) {
        if(f.name[0] == '.')
            return true;

        if(!(f.flags & FileFlags::directory)) {
            std::string ext;
            auto dotPos = f.name.find_last_of('.');
            if(dotPos != std::string::npos)
                ext = f.name.substr(dotPos);

            // convert to lower case
            std::for_each(ext.begin(), ext.end(), [](char & c) {c = tolower(c);});

            if(file_exts.find(ext) == file_exts.end())
                return true;
        }

        return false;
    }), files.end());

    // update menu items
    file_items.resize(files.size());

    unsigned int i = 0;
    for(auto &file : files) {
        if(file.flags & FileFlags::directory)
            file.name += "/";

        file_items[i].id = i;
        file_items[i++].label = file.name.c_str();
    }

    set_items(file_items.data(), file_items.size());
}

void FileBrowser::render_item(const Item &item, int y, int index) const {
    Menu::render_item(item, y, index);

    if(index == current_item) {
        const int iconSize = font.char_h > 8 ? 12 : 8;

        Rect r(display_rect.x + item_padding_x, y, display_rect.w - item_padding_x * 2 - iconSize - 2, item_h);
        Point iconPos = Point(display_rect.x + display_rect.w - item_padding_x -iconSize, y + ((item_h - iconSize) / 2)); // from the top-right
        if (has_focus) control_icon(iconPos, Button::A);
    }
}

void FileBrowser::update_item(const Item &item) {
}

void FileBrowser::item_activated(const Item &item){
    if(!num_items)
        return;

    if(files[current_item].flags & FileFlags::directory) {
        cur_dir += files[current_item].name;
        update_list();
    }
    else if(on_file_open)
        on_file_open(cur_dir + files[current_item].name);
}