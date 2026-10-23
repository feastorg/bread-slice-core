# bread-slice-core

Shared firmware mechanisms for BREADS Slices: the code every Slice runs the
same way, written once and host-tested.

Moving the Slices onto it is tracked in
[feastorg/.github#4](https://github.com/feastorg/.github/issues/4).

## Use

```cpp
#include <bread_slice_core.h>   // e-stop, command watchdog, parsing
#include <bsc_status_led.h>     // only on a board with the status LED
```

[`examples/slice_skeleton`](examples/slice_skeleton/slice_skeleton.ino) shows
the calls in the order a Slice makes them.

### E-stop (`bsc::EStop`)

`EStop(pin, activeLow = true, releaseDebounceMs = 25)`. `begin()` sets the
pin's mode: the internal pull-up when `activeLow`.

`poll(millis())` once per main-loop pass returns `None`, `Pressed` or
`Released`. One active sample is a press. A release needs the line inactive
for the debounce time without a break, and always at least two samples.

`asserted()` and `count()` are what interrupt handlers read. `poll()` sets
them, with interrupts off, before it returns `Pressed`, so commands are
already refused while the Slice drives its outputs safe. The count wraps at
255.

`bsc::bootWait(estop, 1000, onEvent, &watchdog)` samples the line for the
boot wait, passes each edge to `onEvent`, and restamps the watchdog at the end.

### Command watchdog (`bsc::CommandWatchdog`)

- `arm(timeoutMs, millis())` sets the timeout and restarts the timer; 0
  disarms.
- `stamp(millis())` records bus traffic.
- `check(millis())` from the main loop returns true once, on the pass that
  trips. It sets the latch, with interrupts off, before returning.
- `clear()` lifts the latch and nothing else.

Arming, disarming and re-arming leave a trip set. `arm()`, `stamp()` and
`clear()` are safe from an ISR. `tripped()`, `timeoutMs()`, `armed()` and
`tripCount()` (wraps at 255) report its state.

### Status LED (`bsc::StatusLed`)

One WS2812B on D5, on an ATmega328P or ATmega4809 at 16 MHz; anything else
fails to build. `begin()`, then `set(Boot)` (blue), `set(Running)` (green) or
`set(EStop)` (red). A frame goes out only when the state changes; interrupts
are off for about 32 us while it does.

### Strict parsing

`bsc::parseLong(s, lo, hi, &out)` and `bsc::parseFloat(s, lo, hi, &out)`
accept a whole base-10 token, finite and in `[lo, hi]`. Anything else, such as
`"12x"`, `" 1"`, `"0x10"` or `"nan"`, returns false and leaves `out` unchanged.

## Test

```sh
cmake -S tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

CI runs these and builds the examples for the Nano and the Nano Every.

## Scope

The rules for what belongs here are in the BREADS spec,
[Firmware Layering](https://github.com/feastorg/bread-infra/blob/main/spec/firmware-layering.md).
The behaviour this library implements is in
[Slice Firmware Conventions](https://github.com/feastorg/bread-infra/blob/main/spec/slice/firmware.md).
In short:

- **Mechanism, not policy.** Where Slice families differ, this library calls a
  hook the Slice provides. It never decides what a safe state is.
- **No protocol dependency.** It includes neither
  [CRUMBS](https://github.com/feastorg/CRUMBS) nor
  [bread-crumbs-contracts](https://github.com/feastorg/bread-crumbs-contracts).
  The Slice firmware connects a wire command to a function here.
- **Non-blocking.** No call holds the main loop, because the e-stop is sampled
  there.
- **Host-tested.** Every module has host tests. Hardware-specific code fails to
  build, with `#error`, on an MCU it does not support.

Not here: command validation, control loops, device drivers, and opcodes. Those
stay in each Slice, in their own libraries, or in the contracts.

## Licence

[MPL-2.0](LICENSE).
