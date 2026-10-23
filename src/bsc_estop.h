// E-stop input: sampled from the main loop, never by interrupt.
//
// One active sample asserts the stop. Release needs the line inactive
// continuously for the debounce time: the first inactive sample starts the
// timer and any active sample cancels it. The count goes up once per stop.
//
// asserted() and count() are what interrupt handlers read. poll() sets them,
// with interrupts off, before it returns Pressed, so a command that arrives
// while the Slice is still driving its outputs safe is already refused.
// What a stop does is the Slice's: it acts on the returned event.
#ifndef BSC_ESTOP_H
#define BSC_ESTOP_H

#include <stdint.h>

namespace bsc
{

class EStop
{
public:
    enum Event : uint8_t
    {
        None,
        Pressed,
        Released,
    };

    // activeLow: the input uses the internal pull-up and LOW is a stop, as on
    // the BREADS Slices today.
    explicit EStop(uint8_t pin, bool activeLow = true, uint16_t releaseDebounceMs = 25);

    // Configures the pin. Call before the first poll().
    void begin();

    // Reads the pin once and returns the edge, if any.
    Event poll(unsigned long nowMs);

    // As poll(), with the line's state supplied: true is a stop.
    Event update(bool active, unsigned long nowMs);

    bool asserted() const { return asserted_; }
    // Wraps at 255.
    uint8_t count() const { return count_; }

private:
    uint8_t pin_;
    bool activeLow_;
    uint16_t debounceMs_;
    bool releasePending_ = false;
    uint32_t releaseStartMs_ = 0;
    volatile bool asserted_ = false;
    volatile uint8_t count_ = 0;
};

class CommandWatchdog;

// The boot wait: samples the e-stop continuously for durationMs, so a line
// held at power-up is asserted before the main loop runs, and passes every
// edge to onEvent. Then restamps the watchdog, if one is given, so a boot
// timeout shorter than the wait does not trip on the first loop pass.
void bootWait(EStop &estop, uint16_t durationMs, void (*onEvent)(EStop::Event),
              CommandWatchdog *watchdog = nullptr);

} // namespace bsc

#endif
