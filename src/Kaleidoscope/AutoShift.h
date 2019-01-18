// -*- mode: c++ -*-
// Kaleidoscope-AutoShift - Shift a key if it's been held for some time.
// Copyright (C) 2019 Jared Lindsay 
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <Kaleidoscope.h>

namespace kaleidoscope {
namespace plugin {

class AutoShift : public kaleidoscope::Plugin {
  public:
    // Basic plugin status functions.
    static void enable();
    static void disable();
    static bool active();

    static void set_delay(uint16_t new_delay);
    static uint8_t delay();

    EventHandlerResult onKeyswitchEvent(Key &mapped_key, byte row, byte col,
                                        uint8_t key_state);
  
  private:
    static bool disabled_;
    static uint16_t delay_;
    static uint32_t start_time_;

    static inline uint32_t computeTimeDelta(uint32_t start);

    static inline bool isKeyModifier(Key key);
    static inline bool isKeyIgnored(Key key);

    static inline bool isKeyShift(Key key) {
      return (key == Key_LeftShift
              || key == Key_RightShift);
    }

    static inline bool isShiftKeyActive() {
      return (hid::wasModifierKeyActive(Key_LeftShift)
              || hid::wasModifierKeyActive(Key_RightShift));
    }
};


}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::AutoShift AutoShift;
