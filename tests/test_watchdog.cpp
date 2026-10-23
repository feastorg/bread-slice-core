// Command watchdog: disarmed by default, trips once at the timeout, latches
// through arm and disarm, and clear() resumes nothing.
#include <bread_slice_core.h>

#include "check.h"

int main()
{
    // Disarmed by default: never trips.
    {
        bsc::CommandWatchdog wd;
        CHECK(!wd.armed());
        CHECK(!wd.check(100000));
        CHECK(!wd.tripped());
    }

    // Trips at the timeout, not before, and only once.
    {
        bsc::CommandWatchdog wd;
        wd.arm(500, 1000);
        CHECK(wd.armed());
        CHECK(wd.timeoutMs() == 500);
        CHECK(!wd.check(1499));
        CHECK(wd.check(1500));
        CHECK(wd.tripped());
        CHECK(wd.tripCount() == 1);
        CHECK(!wd.check(5000));
        CHECK(wd.tripCount() == 1);
    }

    // A stamp restarts the timer.
    {
        bsc::CommandWatchdog wd;
        wd.arm(500, 0);
        wd.stamp(400);
        CHECK(!wd.check(899));
        CHECK(wd.check(900));
    }

    // The trip latches: re-arming, disarming and stamping leave it set.
    {
        bsc::CommandWatchdog wd;
        wd.arm(100, 0);
        wd.check(100);
        wd.arm(0, 200);
        CHECK(wd.tripped());
        wd.arm(100, 300);
        wd.stamp(350);
        CHECK(wd.tripped());
    }

    // clear() lifts the latch only: the timeout and count stay, and the next
    // trip needs the full timeout from the last stamp.
    {
        bsc::CommandWatchdog wd;
        wd.arm(100, 0);
        wd.check(100);
        wd.stamp(150);
        wd.clear();
        CHECK(!wd.tripped());
        CHECK(wd.armed());
        CHECK(wd.tripCount() == 1);
        CHECK(!wd.check(249));
        CHECK(wd.check(250));
        CHECK(wd.tripCount() == 2);
    }

    // millis() wrap: elapsed time is computed unsigned.
    {
        bsc::CommandWatchdog wd;
        wd.arm(100, 0xFFFFFFC0UL);
        CHECK(!wd.check(0x00000020UL)); // 96 ms later
        CHECK(wd.check(0x00000024UL));  // 100 ms later
    }

    // The count wraps at 255.
    {
        bsc::CommandWatchdog wd;
        wd.arm(1, 0);
        for (int i = 0; i < 256; i++)
        {
            wd.check(1000UL * (i + 1));
            wd.clear();
            wd.stamp(1000UL * (i + 1));
        }
        CHECK(wd.tripCount() == 0);
    }

    CHECK_DONE();
}
