// Strict parsing: whole decimal token, finite, in range; *out untouched on
// failure.
#include <bread_slice_core.h>

#include "check.h"

int main()
{
    long v;
    v = 7;
    CHECK(bsc::parseLong("42", 0, 100, &v) && v == 42);
    CHECK(bsc::parseLong("-255", -255, 255, &v) && v == -255);
    CHECK(bsc::parseLong("+3", 0, 10, &v) && v == 3);

    // Each of these fails and leaves v as it was.
    const char *badLong[] = {
        "", " 1", "1 ", "abc", "12x", "x12", "1.5", "0x10", "--1", "-", "1e3", "1-2",
        "99999999999999999999",
    };
    for (const char *s : badLong)
    {
        v = 7;
        CHECK(!bsc::parseLong(s, -1000000, 1000000, &v));
        CHECK(v == 7);
    }
    v = 7;
    CHECK(!bsc::parseLong(nullptr, 0, 10, &v) && v == 7);

    // Range is inclusive at both ends.
    CHECK(bsc::parseLong("65535", 0, 65535, &v) && v == 65535);
    v = 7;
    CHECK(!bsc::parseLong("65536", 0, 65535, &v) && v == 7);
    CHECK(!bsc::parseLong("-1", 0, 65535, &v) && v == 7);

    float f;
    f = 7;
    CHECK(bsc::parseFloat("25.5", 0, 500, &f) && f == 25.5f);
    CHECK(bsc::parseFloat("-0.25", -1, 1, &f) && f == -0.25f);
    CHECK(bsc::parseFloat("1e2", 0, 500, &f) && f == 100.0f);
    CHECK(bsc::parseFloat("3", 0, 500, &f) && f == 3.0f);

    const char *badFloat[] = {
        "", " 1", "1 ", "abc", "1.5x", "nan", "inf", "-inf", "0x1p3", "1e", ".", "1e999",
        "1.2.3", "1-2",
    };
    for (const char *s : badFloat)
    {
        f = 7;
        CHECK(!bsc::parseFloat(s, -1e30f, 1e30f, &f));
        CHECK(f == 7);
    }
    f = 7;
    CHECK(!bsc::parseFloat("500.1", 0, 500, &f) && f == 7);
    CHECK(!bsc::parseFloat("-0.1", 0, 500, &f) && f == 7);

    CHECK_DONE();
}
