// Fuzzes mintty's base64 decoder (OSC 52 clipboard / OSC 1337 payloads) and checks encode/decode round trips.
#include "base64.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size > (1u << 16))
    return 0;
  int olen = (int)size;
  char *out = malloc(olen + 4);
  base64_decode((const char *)data, (int)size, out, olen);
  base64_decode_clip((const char *)data, (int)size, out, olen);
  free(out);

  int elen = (int)((size + 2) / 3 * 4 + 1);
  char *enc = malloc(elen);
  char *dec = malloc(size + 4);
  int n = base64_encode(data, (int)size, enc, elen);
  if (n >= 0) {
    int m = base64_decode(enc, n, dec, (int)size + 4);
    if (m != (int)size || (size && memcmp(dec, data, size)))
      abort();
  }
  free(enc);
  free(dec);
  return 0;
}
