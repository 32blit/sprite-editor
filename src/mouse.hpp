#pragma once
#include "32blit.hpp"


class AutoRepeat {
  public:
    AutoRepeat(uint32_t repeat_time=200, uint32_t hold_time=1000) {
      this->repeat_time = repeat_time;
      this->hold_time = hold_time;
    }
    bool next(uint32_t time, bool state) {
      bool changed = state != last_state;
      last_state = state;

      if(changed) {
        if(state) {
          pressed_time = time;
          pressed = true;
          last_time = time;
          return true;
        }
        else {
          pressed_time = 0;
          pressed = false;
          last_time = 0;
        }
      }
      // Shortcut for no auto-repeat
      if(repeat_time == 0) return false;

      if(pressed) {
        uint32_t repeat_rate = repeat_time;
        if(hold_time > 0 && time - pressed_time > hold_time) {
          repeat_rate /= 3;
        }
        if(time - last_time > repeat_rate) {
          last_time = time;
          return true;
        }
      }

      return false;
    }
  private:
    uint32_t repeat_time;
    uint32_t hold_time;
    bool pressed = false;
    bool last_state = false;
    uint32_t pressed_time = 0;
    uint32_t last_time = 0;
};

class Mouse {
    public:
        Mouse();
        void render(uint32_t time);
        void update(uint32_t time);
        blit::Vec2 cursor = blit::Vec2(0, 0);
        blit::Vec2 last_cursor = blit::Vec2(0, 0);
        blit::Vec2 dpad;
        bool cursor_moved;

        AutoRepeat ar_dpad_u = AutoRepeat(220, 400);
        AutoRepeat ar_dpad_d = AutoRepeat(220, 400);
        AutoRepeat ar_dpad_l = AutoRepeat(220, 400);
        AutoRepeat ar_dpad_r = AutoRepeat(220, 400);

        AutoRepeat ar_button_a = AutoRepeat(220, 400);
        AutoRepeat ar_button_b = AutoRepeat(220, 400);
        AutoRepeat ar_button_x = AutoRepeat(220, 400);
        AutoRepeat ar_button_y = AutoRepeat(220, 400);
        
        bool dpad_up, dpad_down, dpad_left, dpad_right,
        dpad_up_pressed, dpad_down_pressed, dpad_left_pressed, dpad_right_pressed;
        bool button_a_repeat, button_b_repeat, button_x_repeat, button_y_repeat; 
        bool button_a, button_b, button_x, button_y,
        button_a_pressed, button_b_pressed, button_x_pressed, button_y_pressed;
        bool button_menu, button_menu_pressed;

    private:
};