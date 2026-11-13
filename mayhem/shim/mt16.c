/* 16-bit (UTF-16, as on Cygwin) versions of the wide-string libc calls mintty uses. */
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

typedef char16_t w16;

size_t mt_wcslen(const w16 *s) { size_t n = 0; while (s[n]) n++; return n; }
size_t mt_wcsnlen(const w16 *s, size_t m) { size_t n = 0; while (n < m && s[n]) n++; return n; }
w16 *mt_wcschr(const w16 *s, w16 c) { for (;; s++) { if (*s == c) return (w16 *)s; if (!*s) return 0; } }
w16 *mt_wcsrchr(const w16 *s, w16 c) { const w16 *r = 0; for (;; s++) { if (*s == c) r = s; if (!*s) return (w16 *)r; } }
w16 *mt_wcsncpy(w16 *d, const w16 *s, size_t n) { size_t i = 0; for (; i < n && s[i]; i++) d[i] = s[i]; for (; i < n; i++) d[i] = 0; return d; }
w16 *mt_wcscpy(w16 *d, const w16 *s) { size_t i = 0; do d[i] = s[i]; while (s[i++]); return d; }
w16 *mt_wcsncat(w16 *d, const w16 *s, size_t n) { size_t l = mt_wcslen(d), i = 0; for (; i < n && s[i]; i++) d[l + i] = s[i]; d[l + i] = 0; return d; }
int mt_wcsncmp(const w16 *a, const w16 *b, size_t n) { for (; n; n--, a++, b++) { if (*a != *b) return *a < *b ? -1 : 1; if (!*a) break; } return 0; }
int mt_wcscmp(const w16 *a, const w16 *b) { return mt_wcsncmp(a, b, (size_t)-1); }
w16 *mt_wcsdup(const w16 *s) { size_t n = mt_wcslen(s) + 1; w16 *d = malloc(n * sizeof *d); if (d) memcpy(d, s, n * sizeof *d); return d; }

extern int xcwidth(unsigned);
int mt_wcwidth(unsigned c) { return xcwidth(c); }
int mt_wcswidth(const w16 *s, size_t n)
{
  int w = 0;
  for (size_t i = 0; i < n && s[i]; i++) {
    unsigned c = s[i];
    if ((c & 0xFC00) == 0xD800 && i + 1 < n && (s[i + 1] & 0xFC00) == 0xDC00) {
      c = 0x10000 + ((c & 0x3FF) << 10) + (s[i + 1] & 0x3FF);
      i++;
    }
    int cw = xcwidth(c);
    if (cw < 0) return -1;
    w += cw;
  }
  return w;
}

static size_t utf8_to_16(w16 *d, const char *s, size_t n, size_t *used)
{
  const unsigned char *p = (const unsigned char *)s;
  unsigned c = p[0];
  size_t len = c < 0x80 ? 1 : c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC2 ? 2 : 1;
  if (len > n) len = 1;
  if (len > 1) {
    c &= 0xFF >> (len + 1);
    for (size_t i = 1; i < len; i++) c = (c << 6) | (p[i] & 0x3F);
  }
  *used = len;
  if (c >= 0x10000) {
    if (d) { d[0] = 0xD800 | (((c - 0x10000) >> 10) & 0x3FF); d[1] = 0xDC00 | (c & 0x3FF); }
    return 2;
  }
  if (d) d[0] = (w16)c;
  return 1;
}

size_t mt_mbstowcs(w16 *d, const char *s, size_t n)
{
  size_t out = 0, slen = strlen(s);
  while (slen && *s) {
    size_t used, cnt = utf8_to_16(0, s, slen, &used);
    if (d) { if (out + cnt > n) break; utf8_to_16(d + out, s, slen, &used); }
    out += cnt; s += used; slen -= used;
  }
  if (d && out < n) d[out] = 0;
  return out;
}

size_t mt_mbrtowc(w16 *d, const char *s, size_t n, mbstate_t *st)
{
  (void)st;
  if (!s || !n) return 0;
  size_t used;
  w16 tmp[2];
  utf8_to_16(tmp, s, n, &used);
  if (d) *d = tmp[0];
  return *s ? used : 0;
}
