// E-stop sampling: assert on one active sample, release after the debounce
// time inactive without a break, one count per stop, and the boot wait.
#include <Arduino.h>
#include <bread_slice_core.h>

#include "check.h"

using bsc::EStop;

static int events[8];
static int eventCount;
static void record(EStop::Event e)
{
    if (eventCount < 8)
        events[eventCount++] = e;
}

int main()
{
    // begin(): pull-up for active-low, plain input otherwise.
    {
        EStop e(2);
        e.begin();
        CHECK(fake_pin_mode == INPUT_PULLUP);
        EStop h(2, false);
        h.begin();
        CHECK(fake_pin_mode == INPUT);
    }

    // One active sample asserts and counts.
    {
        EStop e(2);
        CHECK(e.update(true, 0) == EStop::Pressed);
        CHECK(e.asserted());
        CHECK(e.count() == 1);
        CHECK(e.update(true, 1) == EStop::None);
        CHECK(e.count() == 1);
    }

    // Release: the first inactive sample starts the timer; 25 ms later it
    // releases. Nothing resumes the count.
    {
        EStop e(2);
        e.update(true, 0);
        CHECK(e.update(false, 100) == EStop::None);
        CHECK(e.update(false, 124) == EStop::None);
        CHECK(e.asserted());
        CHECK(e.update(false, 125) == EStop::Released);
        CHECK(!e.asserted());
        CHECK(e.count() == 1);
    }

    // An active sample during the debounce restarts it, without a new count.
    {
        EStop e(2);
        e.update(true, 0);
        e.update(false, 10);
        CHECK(e.update(true, 20) == EStop::None);
        CHECK(e.update(false, 30) == EStop::None);
        CHECK(e.update(false, 50) == EStop::None);
        CHECK(e.update(false, 55) == EStop::Released);
        CHECK(e.count() == 1);
        CHECK(e.update(true, 60) == EStop::Pressed);
        CHECK(e.count() == 2);
    }

    // The debounce time is configurable.
    {
        EStop e(2, true, 5);
        e.update(true, 0);
        e.update(false, 10);
        CHECK(e.update(false, 15) == EStop::Released);
    }

    // Release always takes two inactive samples, even with no debounce time.
    {
        EStop e(2, true, 0);
        e.update(true, 0);
        CHECK(e.update(false, 0) == EStop::None);
        CHECK(e.update(false, 0) == EStop::Released);
    }

    // The debounce runs across the millis() wrap.
    {
        EStop e(2);
        e.update(true, 0xFFFFFFF0UL);
        e.update(false, 0xFFFFFFF0UL);
        CHECK(e.update(false, 0x00000008UL) == EStop::None);     // 24 ms later
        CHECK(e.update(false, 0x00000009UL) == EStop::Released); // 25 ms later
    }

    // poll() reads the pin: LOW is a stop when active-low, HIGH otherwise.
    {
        EStop e(2);
        fake_level = HIGH;
        CHECK(e.poll(0) == EStop::None);
        fake_level = LOW;
        CHECK(e.poll(1) == EStop::Pressed);
        EStop h(2, false);
        fake_level = LOW;
        CHECK(h.poll(0) == EStop::None);
        fake_level = HIGH;
        CHECK(h.poll(1) == EStop::Pressed);
    }

    // The count wraps at 255.
    {
        EStop e(2, true, 0);
        for (int i = 0; i < 256; i++)
        {
            e.update(true, 2 * i);
            e.update(false, 2 * i);
            e.update(false, 2 * i + 1);
        }
        CHECK(e.count() == 0);
    }

    // Boot wait: a line held at power-up is pressed during the wait, which
    // lasts its full duration and restamps the watchdog at its end.
    {
        EStop e(2);
        bsc::CommandWatchdog wd;
        wd.arm(500, 0);
        fake_level = LOW;
        fake_now = 0;
        fake_step = 1;
        eventCount = 0;
        bsc::bootWait(e, 1000, record, &wd);
        CHECK(eventCount == 1 && events[0] == EStop::Pressed);
        CHECK(e.asserted());
        CHECK(fake_now >= 1000);
        // Stamped by the wait's last millis() read, one step before fake_now:
        // no trip until 500 ms after that.
        const unsigned long stampedAt = fake_now - 1;
        CHECK(stampedAt >= 1000);
        CHECK(!wd.check(stampedAt + 499));
        CHECK(wd.check(stampedAt + 500));
    }

    // Boot wait with the line released: no events, no callback needed.
    {
        EStop e(2);
        fake_level = HIGH;
        fake_now = 0;
        bsc::bootWait(e, 1000, nullptr);
        CHECK(!e.asserted());
    }

    CHECK_DONE();
}
