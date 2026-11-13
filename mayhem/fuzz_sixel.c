// Feeds arbitrary bytes to mintty's DECSIXEL image parser, as the terminal does for a DCS q ... ST body.
#include "sixel.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size < 1 || size > (1u << 20))
    return 0;
  sixel_state_t *st = calloc(1, sizeof *st);
  if (!st)
    return 0;
  int private_regs = data[0] & 1;
  unsigned char *body = malloc(size);
  memcpy(body, data + 1, size - 1);
  if (sixel_parser_init(st, 0xFFFFFF, 0x000000, private_regs) >= 0) {
    if (sixel_parser_parse(st, body, (int)(size - 1)) >= 0) {
      unsigned char *pixels = sixel_parser_finalize(st);
      free(pixels);
    }
    sixel_parser_deinit(st);
  }
  free(body);
  free(st);
  return 0;
}
