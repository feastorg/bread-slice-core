// Internal: a block that runs with interrupts off and restores the previous
// state, so a call is safe both from the main loop and from an ISR.
#ifndef BSC_ATOMIC_H
#define BSC_ATOMIC_H

#if defined(__AVR__)
#include <util/atomic.h>
#define BSC_ATOMIC ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#else
// Host builds run single-threaded tests.
#define BSC_ATOMIC for (int bsc_once_ = 1; bsc_once_; bsc_once_ = 0)
#endif

#endif
