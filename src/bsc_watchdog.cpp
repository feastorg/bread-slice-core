#include "bsc_watchdog.h"

#include "bsc_atomic.h"

namespace bsc
{

void CommandWatchdog::arm(uint16_t timeoutMs, unsigned long nowMs)
{
    BSC_ATOMIC
    {
        timeoutMs_ = timeoutMs;
        lastStampMs_ = nowMs;
    }
}

void CommandWatchdog::stamp(unsigned long nowMs)
{
    BSC_ATOMIC
    {
        lastStampMs_ = nowMs;
    }
}

bool CommandWatchdog::check(unsigned long nowMs)
{
    bool trip = false;
    // The comparison runs in the same window as the latch, so a stamp from
    // an ISR can't land between the two.
    BSC_ATOMIC
    {
        // Elapsed time in 32 bits, the width of millis() on AVR, so it stays
        // right across the 49-day wrap on any host.
        if (timeoutMs_ != 0 && !tripped_ && (uint32_t)(nowMs - lastStampMs_) >= timeoutMs_)
        {
            tripped_ = true;
            tripCount_++;
            trip = true;
        }
    }
    return trip;
}

void CommandWatchdog::clear()
{
    tripped_ = false;
}

bool CommandWatchdog::armed() const
{
    return timeoutMs() != 0;
}

uint16_t CommandWatchdog::timeoutMs() const
{
    uint16_t t;
    BSC_ATOMIC
    {
        t = timeoutMs_;
    }
    return t;
}

} // namespace bsc
