// Fuzzes mintty's mini bidi implementation (UAX #9 reordering and Arabic shaping) over arbitrary code points.
#include "std.h"
#include "minibidi.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size < 3)
    return 0;
  bool autodir = data[0] & 1, explicit_rtl = data[0] & 2, box_mirror = data[0] & 4;
  int para_level = (data[0] >> 3) & 1;
  size_t count = (size - 1) / 2;
  if (count > 4096)
    count = 4096;
  bidi_char *line = calloc(count, sizeof *line);
  bidi_char *shaped = calloc(count, sizeof *shaped);
  for (size_t i = 0; i < count; i++) {
    unsigned cp = data[1 + 2 * i] | (data[2 + 2 * i] << 8);
    line[i].origwc = line[i].wc = cp;
    line[i].index = (short)i;
  }
  do_bidi(autodir, para_level, explicit_rtl, box_mirror, line, (int)count);
  do_shape(line, shaped, (int)count);
  free(line);
  free(shaped);
  return 0;
}
