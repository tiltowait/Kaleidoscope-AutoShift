# Kaleidoscope-AutoShift

A simple plugin that sends a shifted version of a key if that key has been held for long enough.

## Using the Plugin

Installation of `AutoShift` into your Kaleidoscope sketch is simple.

```c++
#include <Kaleidoscope.h>
#include <Kaleidoscope-AutoShift.h>

KALEIDOSCOPE_INIT_PLUGINS(AutoShift);

void setup() {
  Kaleidoscope.setup();
}
```

## Configuring the Plugin

`AutoShift` has three member functions for controlling its behavior.

### `enable()`

Enables the plugin.

### `disable()`

Disables the plugin.

### `set_delay(uint16_t new_delay)`

Change the shift-key delay to `new_delay`. This is a positive integer value that represents time in milliseconds. By default, the delay is set to 500ms.

## Querying the Plugin

The plugin also has two member functions for determining its state.

### `active()`

Returns `true` if the plugin is enabled.

### `delay()`

Returns an unsigned, 16-bit integer representing the current shift-key delay period, in milliseconds.