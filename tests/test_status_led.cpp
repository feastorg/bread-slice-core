// Status LED: each state's colour in GRB order, written only on a change, and
// the main loop's choice of state.
#include <bsc_status_led.h>

#include "check.h"

using bsc::StatusLed;

static bool frame(uint8_t g, uint8_t r, uint8_t b)
{
    return bsc_host_led_frame[0] == g && bsc_host_led_frame[1] == r && bsc_host_led_frame[2] == b;
}

int main()
{
    StatusLed led;
    led.begin();
    CHECK(bsc_host_led_writes == 0);

    led.set(StatusLed::Boot);
    CHECK(bsc_host_led_writes == 1 && frame(0, 0, 50));

    led.set(StatusLed::Running);
    CHECK(bsc_host_led_writes == 2 && frame(25, 0, 0));
    led.set(StatusLed::Running);
    CHECK(bsc_host_led_writes == 2);

    led.set(StatusLed::EStop);
    CHECK(bsc_host_led_writes == 3 && frame(0, 50, 0));
    led.set(StatusLed::EStop);
    CHECK(bsc_host_led_writes == 3);

    led.set(StatusLed::Running);
    CHECK(bsc_host_led_writes == 4 && frame(25, 0, 0));

    // Amber: red 50, green 16, no blue.
    led.set(StatusLed::Hold);
    CHECK(bsc_host_led_writes == 5 && frame(16, 50, 0));
    led.set(StatusLed::Hold);
    CHECK(bsc_host_led_writes == 5);

    // Red wins over amber; amber only while held without the e-stop.
    CHECK(StatusLed::stateFor(false, false) == StatusLed::Running);
    CHECK(StatusLed::stateFor(false, true) == StatusLed::Hold);
    CHECK(StatusLed::stateFor(true, false) == StatusLed::EStop);
    CHECK(StatusLed::stateFor(true, true) == StatusLed::EStop);

    CHECK_DONE();
}
