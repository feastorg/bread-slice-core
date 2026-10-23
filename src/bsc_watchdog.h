// Command watchdog: a timer the controller arms, refreshed by bus traffic,
// that trips when the bus goes silent. A trip latches until clear(); arming,
// disarming and re-arming leave it set, and clearing resumes nothing.
//
// What refreshes it is defined in bread-crumbs-contracts (bread_watchdog.h),
// and what a trip does is the Slice's: it acts when check() returns true.
//
// arm(), stamp() and clear() are safe from an ISR and from the main loop.
// check() is for the main loop. It sets the latch, with interrupts off,
// before it returns true, so a command arriving while the Slice drives its
// outputs safe is already refused.
#ifndef BSC_WATCHDOG_H
#define BSC_WATCHDOG_H

#include <stdint.h>

namespace bsc
{

class CommandWatchdog
{
public:
    // Sets the timeout and restarts the timer. 0 disarms. Does not clear a
    // trip.
    void arm(uint16_t timeoutMs, unsigned long nowMs);

    // Records bus traffic.
    void stamp(unsigned long nowMs);

    // True once, on the pass that trips: armed, not already tripped, and at
    // least the timeout since the last stamp.
    bool check(unsigned long nowMs);

    void clear();

    bool armed() const;
    uint16_t timeoutMs() const;
    bool tripped() const { return tripped_; }
    // Wraps at 255.
    uint8_t tripCount() const { return tripCount_; }

private:
    volatile uint16_t timeoutMs_ = 0;
    volatile uint32_t lastStampMs_ = 0;
    volatile bool tripped_ = false;
    volatile uint8_t tripCount_ = 0;
};

} // namespace bsc

#endif
