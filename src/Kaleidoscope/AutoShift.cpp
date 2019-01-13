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

// When a key is toggled on, start a timer. If the key remains held longer than
// delay_, send a shifted version of that key and halt keyswitch events until
// the key is released. If the key is released within the time window, send
// that key as-is.
EventHandlerResult AutoShift::onKeyswitchEvent(Key &mapped_key, byte row,
                                              byte col, uint8_t key_state) {
  // Since modifiers are held by design, we don't enforce any special rules on
  // them. Also, allow backspace to be spammed, because dear God did I just
  // learn how annoying it is not to have that functionality.
  if(disabled_
     || mapped_key == Key_Backspace
     || isKeyModifier(mapped_key)) {
    return EventHandlerResult::OK;
  }

  // Start the timer when the user depresses a key.
  if(keyToggledOn(key_state) && mapped_key.flags ^ SHIFT_HELD) {
    start_time_ = Kaleidoscope.millisAtCycleStart();
    return EventHandlerResult::EVENT_CONSUMED;  // We only send OK on release
  }

  // When the user releases a key, send that key if it's within the time period.
  if(keyToggledOff(key_state)) {
    // Determine how long the key's been held.
    uint32_t delta = computeTimeDelta(start_time_);
    start_time_   = 0;  // Reset the timer.

    // If we're within the delay period, send the key as-is.
    if(delta <= delay_) {
      hid::pressKey(mapped_key);
      return EventHandlerResult::OK;
    }
  }
  
  // Allow the key to go through if it's shifted.
  else if(key_state & SHIFT_HELD) {
    return EventHandlerResult::OK;
  }

  // If it's being held, check if it's time to shift it.
  if(keyWasPressed(key_state) && start_time_ != 0) {
    // Determine how long the key's been held.
    uint32_t delta = computeTimeDelta(start_time_);

    // We passed the time window, so shift the key.
    if(delta > delay_) {
      hid::pressKey(LSHIFT(mapped_key));
      start_time_ = 0;  // Reset the timer.
      return EventHandlerResult::OK;
    }
  }

  // We haven't passed the delay period, and a key hasn't been released, so
  // don't do anything.
  return EventHandlerResult::EVENT_CONSUMED;
}

uint32_t AutoShift::computeTimeDelta(uint32_t start) {
  uint32_t current_time = Kaleidoscope.millisAtCycleStart();
  return current_time - start;
}
// Returns true if key is control, alt, shift, or gui.
bool AutoShift::isKeyModifier(Key key) {
  // If it's not a keyboard key, return false
  if(key.flags & (SYNTHETIC | RESERVED)) return false;

  return (key.keyCode >= HID_KEYBOARD_FIRST_MODIFIER &&
          key.keyCode <= HID_KEYBOARD_LAST_MODIFIER);
}

// Legacy V1 API.
#if KALEIDOSCOPE_ENABLE_V1_PLUGIN_API
void AutoShift::begin() {
  Kaleidoscope.useEventHandlerHook(legacyEventHandler);
}

Key AutoShift::legacyEventHandler(Key mapped_key, byte row, byte col, uint8_t keyState) {
  EventHandlerResult r = ::AutoShift.onKeyswitchEvent(mapped_key, row, col, keyState);
  if (r == EventHandlerResult::OK)
    return mapped_key;
  return Key_NoKey;
}
#endif

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::AutoShift AutoShift;