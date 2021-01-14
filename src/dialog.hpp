#pragma once

#include "engine/engine.hpp"
#include "mouse.hpp"
#include "control-icons.hpp"

struct Dialog {
  std::string title, message;

  using AnswerFunc = void(*)(bool);
  AnswerFunc on_answer;

  blit::Rect dialog_rect = blit::Rect(45, 77, 230, 85);
  const blit::Rect rect_y = blit::Rect(blit::Point(dialog_rect.x + 170, dialog_rect.y + 70), blit::Size(24, 10));
  const blit::Rect rect_x = blit::Rect(blit::Point(dialog_rect.x + 201, dialog_rect.y + 70), blit::Size(25, 10));
  const int header_height = 16;

  void show(std::string title, std::string message, AnswerFunc on_answer) {
    this->title = title;
    this->message = message;
    this->on_answer = on_answer;
  }

  bool update(Mouse *mouse) {
    if(title.empty())
      return false;

    bool button_x = mouse->button_x_pressed;
    bool button_y = mouse->button_y_pressed;

    if(mouse->button_a_pressed) {
        if(rect_x.contains(mouse->cursor)) button_x = true;
        if(rect_y.contains(mouse->cursor)) button_y = true;
    }

    if(button_x) {
      on_answer(true);
      title = message = "";
    } else if(button_y) {
      on_answer(false);
      title = message = "";
    }

    return true;
  }

  void draw(Mouse *mouse) {
    if(title.empty())
      return;

    blit::screen.pen = blit::Pen(0, 0, 0, 200);
    blit::screen.rectangle(blit::Rect(blit::Point(0, 0), blit::screen.bounds));

    blit::screen.pen = blit::Pen(235, 245, 255);
    blit::screen.rectangle(dialog_rect);

    blit::screen.pen = blit::Pen(3, 5, 7);
    blit::screen.rectangle(blit::Rect(dialog_rect.x + 1, dialog_rect.y + header_height, dialog_rect.w - 2, dialog_rect.h - header_height - 1));

    blit::screen.text(title, blit::minimal_font, blit::Rect(dialog_rect.x + 3, dialog_rect.y, dialog_rect.w - 6, header_height + blit::minimal_font.spacing_y), true, blit::TextAlign::center_left);

    if(rect_x.contains(mouse->cursor)) {
        blit::screen.pen = blit::Pen(50, 50, 50, 255);
        blit::screen.rectangle(rect_x);
    }
    else if(rect_y.contains(mouse->cursor)) {
        blit::screen.pen = blit::Pen(50, 50, 50, 255);
        blit::screen.rectangle(rect_y);
    }

    blit::screen.pen = blit::Pen(255, 255, 255);
    blit::screen.text(message, blit::minimal_font, blit::Rect(dialog_rect.x + 6, dialog_rect.y + header_height + 5, dialog_rect.w - 12, 45));

    blit::screen.text("No      Yes    ", blit::minimal_font, blit::Rect(dialog_rect.x + 1, dialog_rect.y + dialog_rect.h - 17, dialog_rect.w - 2, 16 + blit::minimal_font.spacing_y), true, blit::TextAlign::center_right);

    control_icon(blit::Point(dialog_rect.x + 185, dialog_rect.y + 71), blit::Button::Y);
    control_icon(blit::Point(dialog_rect.x + 218, dialog_rect.y + 71), blit::Button::X);
  }

};