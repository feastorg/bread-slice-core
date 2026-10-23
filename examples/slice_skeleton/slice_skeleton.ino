// The core's calls in the order a Slice makes them. The safe-state actions
// are placeholders: what a stop or a trip does is each Slice's own.
#include <avr/wdt.h>
#include <bread_slice_core.h>
#include <bsc_status_led.h>

static bsc::EStop estop(2);
static bsc::CommandWatchdog watchdog;
static bsc::StatusLed led;

static void driveOutputsSafe() {}

static void onEStop(bsc::EStop::Event e)
{
    if (e == bsc::EStop::Pressed)
        driveOutputsSafe();
    // Released: nothing resumes until the controller sends new commands.
}

void setup()
{
    wdt_disable();
    estop.begin();
    led.begin();
    led.set(bsc::StatusLed::Boot);
    Serial.begin(115200);
    // A bus receive handler would call watchdog.stamp(millis()) per frame,
    // and SET_WATCHDOG would call watchdog.arm(ms, millis()).
    bsc::bootWait(estop, 1000, onEStop, &watchdog);
    wdt_enable(WDTO_1S);
}

void loop()
{
    wdt_reset();
    onEStop(estop.poll(millis()));
    led.set(estop.asserted() ? bsc::StatusLed::EStop : bsc::StatusLed::Running);
    if (watchdog.check(millis()))
        driveOutputsSafe();

    static char line[16];
    static uint8_t n;
    while (Serial.available())
    {
        const char c = (char)Serial.read();
        if (c != '\n' && n < sizeof line - 1)
        {
            line[n++] = c;
            continue;
        }
        line[n] = '\0';
        n = 0;
        long ms;
        if (bsc::parseLong(line, 0, 65535, &ms))
            watchdog.arm((uint16_t)ms, millis());
        else
            Serial.println(F("Ignored: not a number in 0..65535"));
        break; // one line per loop pass
    }
}
