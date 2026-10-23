# Changelog

Format: [Keep a Changelog](https://keepachangelog.com/). Versions follow
[semver](https://semver.org/); 0.x minors may break, and say so here.

## [Unreleased]

### Added
- Repository skeleton: README, scope rules (linked to the BREADS spec), MPL-2.0
  licence, and the PlatformIO manifest.
- `bsc::EStop`: e-stop sampling with assert on one active sample, a
  25 ms release debounce, a press count, and `bsc::bootWait()`, which samples
  through the boot wait and restamps the command watchdog when it ends.
  Taken from Slice_DCMT and Slice_RLHT, which share the same logic; the
  restamp is new.
- `bsc::CommandWatchdog`: the timer and trip latch from both Slices. The
  timeout comparison now runs in the same interrupt-masked window as the
  latch.
- `bsc::StatusLed`: the single-pixel WS2812B driver from
  feastorg/Slice_DCMT#55, benched on a gen2 Nano and Nano Every on
  2026-10-20.
- `bsc::parseLong()` and `bsc::parseFloat()`: strict whole-token parsing for
  serial consoles.
- Host tests for every module, and CI that runs them and builds the example
  for the Nano and the Nano Every.
