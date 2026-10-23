#include "bsc_estop.h"

#include <Arduino.h>

#include "bsc_atomic.h"
#include "bsc_watchdog.h"

namespace bsc
{

EStop::EStop(uint8_t pin, bool activeLow, uint16_t releaseDebounceMs)
    : pin_(pin), activeLow_(activeLow), debounceMs_(releaseDebounceMs)
{
}

void EStop::begin()
{
    pinMode(pin_, activeLow_ ? INPUT_PULLUP : INPUT);
}

EStop::Event EStop::poll(unsigned long nowMs)
{
    const bool low = digitalRead(pin_) == LOW;
    return update(activeLow_ ? low : !low, nowMs);
}

EStop::Event EStop::update(bool active, unsigned long nowMs)
{
    if (active)
    {
        releasePending_ = false;
        if (asserted_)
            return None;
        BSC_ATOMIC
        {
            asserted_ = true;
            count_++;
        }
        return Pressed;
    }
    if (!asserted_)
        return None;
    if (!releasePending_)
    {
        releasePending_ = true;
        releaseStartMs_ = nowMs;
        return None;
    }
    if ((uint32_t)(nowMs - releaseStartMs_) < debounceMs_)
        return None;
    releasePending_ = false;
    asserted_ = false;
    return Released;
}

void bootWait(EStop &estop, uint16_t durationMs, void (*onEvent)(EStop::Event),
              CommandWatchdog *watchdog)
{
    const unsigned long startMs = millis();
    unsigned long nowMs = startMs;
    while ((uint32_t)(nowMs - startMs) < durationMs)
    {
        const EStop::Event e = estop.poll(nowMs);
        if (e != EStop::None && onEvent != nullptr)
            onEvent(e);
        nowMs = millis();
    }
    if (watchdog != nullptr)
        watchdog->stamp(millis());
}

} // namespace bsc
