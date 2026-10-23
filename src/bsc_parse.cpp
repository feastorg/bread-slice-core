#include "bsc_parse.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>

namespace bsc
{

// strtol and strtod skip leading whitespace; a token must not have any.
// strtod also takes hex floats on some C libraries but not avr-libc, so a
// token is limited to the characters of a decimal number. "inf" and "nan"
// fail on their letters.
static bool decimalToken(const char *s, bool allowPoint)
{
    if (s == nullptr || *s == '\0')
        return false;
    for (const char *p = s; *p != '\0'; p++)
    {
        const char c = *p;
        if ((c >= '0' && c <= '9') || c == '-' || c == '+')
            continue;
        if (allowPoint && (c == '.' || c == 'e' || c == 'E'))
            continue;
        return false;
    }
    return true;
}

bool parseLong(const char *s, long lo, long hi, long *out)
{
    if (!decimalToken(s, false))
        return false;
    char *end;
    errno = 0;
    const long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v < lo || v > hi)
        return false;
    *out = v;
    return true;
}

bool parseFloat(const char *s, float lo, float hi, float *out)
{
    if (!decimalToken(s, true))
        return false;
    char *end;
    errno = 0;
    const double d = strtod(s, &end);
    if (end == s || *end != '\0' || errno == ERANGE || !isfinite(d))
        return false;
    const float v = (float)d;
    if (!isfinite(v) || v < lo || v > hi)
        return false;
    *out = v;
    return true;
}

} // namespace bsc
