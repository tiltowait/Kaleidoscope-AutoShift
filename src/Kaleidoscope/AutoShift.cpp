// -*- mode: c++ -*-
// Kaleidoscope-AutoShift - Shift a key if it's been held for some time.
// Copyright (C) 2019  Jared Lindsay
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

#include <Kaleidoscope-AutoShift.h>

namespace kaleidoscope {
namespace plugin {

// AutoShift

// Member variables.
bool AutoShift::disabled_ = false;  // Keeps track of plugin's active state

// Time-related variables.
uint16_t AutoShift::delay_        = 500;  // The timeout, in ms.
uint32_t AutoShift::start_time_   = 0;    // Start time registered for each
                                          // keystroke.

// Basic plugin status functions.

// Enable the plugin.
void AutoShift::enable() {
  disabled_ = false;
}

// Disable the plugin.
void AutoShift::disable() {
  disabled_ = true;
}

// Returns true if the plugin is enabled.
bool AutoShift::active() {
  return !disabled_;
}

// Set a new delay for injecting shifted keys.
void AutoShift::set_delay(uint16_t new_delay) {
  delay_ = new_delay;
}

// Get the current shifted key injection delay.
uint8_t AutoShift::delay() {
  return delay_;
}

// Event handlers.

// When a key is toggled on, send it and start a timer. If the key remains held
// longer than delay_, delete it, send a shifted version of that key, and halt
// keyswitch events until the key is released.
EventHandlerResult AutoShift::onKeyswitchEvent(Key &mapped_key, byte row,
                                              byte col, uint8_t key_state) {
  // Since modifiers are held by design, we don't enforce any special rules on
  // them. Also, allow backspace to be spammed, because dear God did I just
  // learn how annoying it is not to have that functionality.
  if(disabled_ || isKeyIgnored(mapped_key)) {
    return EventHandlerResult::OK;
  }

  // When the user presses a key, send that key and start the timer.
  if(keyToggledOn(key_state) && mapped_key.flags ^ SHIFT_HELD) {
    start_time_ = Kaleidoscope.millisAtCycleStart();
    return EventHandlerResult::OK;
  }

  // User accepted whatever key was output, so reset the timer.
  if(keyToggledOff(key_state)) {
    start_time_ = 0;
  }
  
  // If it's being held, check if it's time to shift it.
  if(keyWasPressed(key_state) && start_time_ != 0) {
    // Determine how long the key's been held.
    uint32_t delta = computeTimeDelta(start_time_);

    // We passed the time window, so shift the key.
    if(delta > delay_) {
      // First, delete the lowercase keystroke already issued.
      hid::pressKey(Key_Backspace);

      // We aren't using LSHIFT(mapped_key) because that gives us a conversion
      // warning.
      Key shifted_key = mapped_key;
      shifted_key.flags |= SHIFT_HELD;
      hid::pressKey(shifted_key);
      start_time_ = 0;  // Reset the timer.
      return EventHandlerResult::OK;
    }
  }

  // We need to disable key repeat or we'll wind up with tons of lowercase
  // letters, followed by tons of uppercase ones.
  return EventHandlerResult::EVENT_CONSUMED;
}

uint32_t AutoShift::computeTimeDelta(uint32_t start) {
  uint32_t current_time = Kaleidoscope.millisAtCycleStart();
  return current_time - start;
}

// Certain keys are unaffected by shift and should be ignored by the plugin.
// This has the benefit of allowing them to repeat, which is nice fof backspace,
// arrow keys, etc.
//
// Luckily, the keycodes are almost all sequential, so we can do a simple
// comparison:
//   Home:   0x4A
//   PgUp:   0x4B
//   Delete: 0x4C
//   End:    0x4D
//   PgDn:   0x4E
//   Right:  0x4F
//   Left:   0x50
//   Down:   0x51
//   Up:     0x52
//

#define FIRST_IGNORED_KEY 0x4A
#define LAST_IGNORED_KEY  0x52

// Doing it this way allows us to potentially save a few comparisons. I've
// tried to arrange them by decreasing prevalence so as to reduce the average
// number of lookups.

// In addition to the above, we have to ignore modifiers, which already have a
// defined range: HID_KEYBOARD_FIRST_MODIFIER and HID_KEYBOARD_LAST_MODIFIER.
//
// Finally, we have some one-offs:
//   Key_Backspace
//   Key_NoKey
//   Key_skip
// key.flags & SYNTHETIC
bool AutoShift::isKeyIgnored(Key key) {
  return key == Key_Backspace
         || (key.keyCode >= HID_KEYBOARD_FIRST_MODIFIER
             && key.keyCode <= HID_KEYBOARD_LAST_MODIFIER)
         || (key.keyCode >= FIRST_IGNORED_KEY
             && key.keyCode <= LAST_IGNORED_KEY)
         || key.flags & SYNTHETIC
         || key == Key_Transparent
         || key == Key_NoKey;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::AutoShift AutoShift;
