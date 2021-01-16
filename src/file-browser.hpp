#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "engine/file.hpp"
#include "engine/menu.hpp"
#include "mouse.hpp"


class FileBrowser final : public blit::Menu {
public:
    FileBrowser(const blit::Font &font = blit::minimal_font);

    void init();

    void render();

    bool update(uint32_t time, Mouse *mouse);

    void set_extensions(std::set<std::string> exts);

    void set_on_file_open(void (*func)(std::string));

    void set_current_dir(const std::string &dir);

    void set_height(uint32_t height);

private:
    void update_list();

    void render_item(const Item &item, int y, int index) const override;

    void update_item(const Item &item) override;

    void item_activated(const Item &item) override;

    bool has_focus = false;
    std::vector<blit::FileInfo> files;
    std::vector<Item> file_items;
    std::string cur_dir = "/";

    std::set<std::string> file_exts;
    void (*on_file_open)(std::string) = nullptr;
};