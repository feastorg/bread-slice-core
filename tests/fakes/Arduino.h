// Host stand-in for the parts of Arduino.h the library uses.
#ifndef FAKE_ARDUINO_H
#define FAKE_ARDUINO_H

#include <stdint.h>

#define LOW 0
#define HIGH 1
#define INPUT 0
#define INPUT_PULLUP 2

// The level digitalRead() returns for every pin, and the last pinMode().
extern int fake_level;
extern int fake_pin_mode;

// millis() returns fake_now, then adds fake_step, so a loop that polls the
// clock advances it.
extern unsigned long fake_now;
extern unsigned long fake_step;

inline unsigned long millis()
{
    const unsigned long t = fake_now;
    fake_now += fake_step;
    return t;
}
inline int digitalRead(uint8_t) { return fake_level; }
inline void pinMode(uint8_t, uint8_t mode) { fake_pin_mode = mode; }

#endif
