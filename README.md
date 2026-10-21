# bread-slice-core

Shared firmware mechanisms for BREADS Slices: the code every Slice runs the
same way, written once and host-tested.

Nothing is released yet. v0.1.0 is planned to hold:

- the status LED driver (one WS2812B on D5, ATmega328P and ATmega4809 at 16 MHz);
- e-stop sampling, release debounce, press count and the boot wait;
- the command-watchdog timer and trip latch, with a per-family trip hook;
- strict number parsing for serial consoles.

Progress is tracked in
[feastorg/.github#4](https://github.com/feastorg/.github/issues/4).

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
