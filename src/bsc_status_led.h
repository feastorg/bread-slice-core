// The BREADS status LED: one WS2812B on D5, GRB order, 800 kHz. Include this
// header only on a board that has the LED: it drives D5 as an output, and on
// some boards D5 is another signal (Slice_DCMT gen1 uses it for motor 1's
// direction).
//
// The bit loop is cycle-counted at 16 MHz. Per bit: 20 cycles (1.25 us),
// high for 5 cycles (312 ns) for a 0 and 12 cycles (750 ns) for a 1, inside
// the WS2812B windows of 250-550 ns and 650-950 ns. out, nop, sbrs, lsl, dec
// and brne take the same cycles on the ATmega328P (AVRe) and the ATmega4809
// (AVRxt), so one loop serves both; st would not, which is why the port is
// written with out.
//
// Interrupts are off for a frame, about 32 us. StatusLed writes only when the
// state changes.
#ifndef BSC_STATUS_LED_H
#define BSC_STATUS_LED_H

#include <stdint.h>

#if defined(BSC_HOST_TEST)
// Host tests: frames are recorded instead of sent.
inline uint8_t bsc_host_led_frame[3];
inline int bsc_host_led_writes;
#else
#include <Arduino.h>

#if F_CPU != 16000000L
#error "bsc_status_led.h is cycle-counted for 16 MHz"
#endif

#if defined(__AVR_ATmega4809__)
#define BSC_STATUS_LED_OUT VPORTB_OUT // D5 is PB2 on the Nano Every
#define BSC_STATUS_LED_DIR VPORTB_DIR
#define BSC_STATUS_LED_BIT 2
#elif defined(__AVR_ATmega328P__)
#define BSC_STATUS_LED_OUT PORTD // D5 is PD5 on the Nano
#define BSC_STATUS_LED_DIR DDRD
#define BSC_STATUS_LED_BIT 5
#else
#error "bsc_status_led.h supports the ATmega328P and ATmega4809 only"
#endif
#endif

namespace bsc
{

class StatusLed
{
public:
    enum State : uint8_t
    {
        Boot,    // blue, through the boot wait
        Running, // green
        EStop,   // red, while the e-stop is held
    };

    // Drives D5 low as an output.
    void begin()
    {
#if !defined(BSC_HOST_TEST)
        BSC_STATUS_LED_OUT &= (uint8_t)~_BV(BSC_STATUS_LED_BIT);
        BSC_STATUS_LED_DIR |= _BV(BSC_STATUS_LED_BIT);
#endif
    }

    // Shows the state's colour, if it differs from the last one shown.
    void set(State s)
    {
        if (s == last_)
            return;
        last_ = s;
        switch (s)
        {
        case Boot:
            show(0, 0, 50);
            break;
        case Running:
            show(0, 25, 0);
            break;
        case EStop:
            show(50, 0, 0);
            break;
        }
    }

    // Sends one colour, unconditionally.
    static void show(uint8_t r, uint8_t g, uint8_t b)
    {
        const uint8_t frame[3] = {g, r, b};
#if defined(BSC_HOST_TEST)
        for (uint8_t i = 0; i < 3; i++)
            bsc_host_led_frame[i] = frame[i];
        bsc_host_led_writes++;
#else
        const uint8_t sreg = SREG;
        cli();
        // Read the port with interrupts off, so the frame rewrites the other
        // pins on it with the values they already hold.
        const uint8_t hi = BSC_STATUS_LED_OUT | _BV(BSC_STATUS_LED_BIT);
        const uint8_t lo = BSC_STATUS_LED_OUT & (uint8_t)~_BV(BSC_STATUS_LED_BIT);
        for (uint8_t i = 0; i < 3; i++)
        {
            uint8_t v = frame[i];
            uint8_t n;
            asm volatile(
                "ldi  %[n], 8\n"
                "1:\n\t"
                "out  %[port], %[hi]\n\t" // 0: high
                "nop\n\tnop\n\tnop\n\t"
                "sbrs %[v], 7\n\t"
                "out  %[port], %[lo]\n\t" // 5: low for a 0 bit
                "lsl  %[v]\n\t"
                "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
                "out  %[port], %[lo]\n\t" // 12: low for a 1 bit
                "nop\n\tnop\n\tnop\n\tnop\n\t"
                "dec  %[n]\n\t"
                "brne 1b\n\t" // 20: next bit
                : [v] "+r"(v), [n] "=&d"(n)
                : [port] "I"(_SFR_IO_ADDR(BSC_STATUS_LED_OUT)), [hi] "r"(hi), [lo] "r"(lo));
        }
        SREG = sreg;
#endif
    }

private:
    uint8_t last_ = 0xFF;
};

} // namespace bsc

#endif
