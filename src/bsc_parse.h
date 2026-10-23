// Strict number parsing for serial consoles. The whole token must parse as a
// base-10 number, with no leading or trailing characters, and the value must
// be finite and in [lo, hi]. On failure the functions return false and leave
// *out unchanged, so a typo changes nothing.
#ifndef BSC_PARSE_H
#define BSC_PARSE_H

namespace bsc
{

bool parseLong(const char *s, long lo, long hi, long *out);
bool parseFloat(const char *s, float lo, float hi, float *out);

} // namespace bsc

#endif
