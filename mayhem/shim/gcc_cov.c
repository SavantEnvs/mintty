/* gcc only offers -fsanitize-coverage=trace-pc, whose callback libFuzzer rejects. build.sh renames that
   callback in the gcc objects to mt_cov_trace_pc, which feeds libFuzzer's inline-8bit-counters coverage:
   each call site's return address hashes into a counter table registered at startup. Built WITHOUT
   coverage flags. */
#include <stdint.h>

#define NCOUNTERS (1u << 16)
static uint8_t counters[NCOUNTERS];

void __sanitizer_cov_8bit_counters_init(uint8_t *start, uint8_t *stop) __attribute__((weak));

__attribute__((constructor)) static void mt_cov_init(void)
{
  if (__sanitizer_cov_8bit_counters_init)
    __sanitizer_cov_8bit_counters_init(counters, counters + NCOUNTERS);
}

void mt_cov_trace_pc(void)
{
  uintptr_t pc = (uintptr_t)__builtin_return_address(0);
  uint8_t *c = &counters[((uint32_t)((pc >> 1) * 2654435761u) >> 16)];
  if (*c != 255)
    ++*c;
}
