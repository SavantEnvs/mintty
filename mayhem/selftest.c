// Known-answer checks over mintty's headless terminal core, base64, sixel and bidi code.
// Built with the project's normal flags (no sanitizers) and run by mayhem/test.sh.
#include "term_init.h"
#include "base64.h"
#include "sixel.h"
#include "minibidi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed, failed;

static void check(const char *name, bool ok)
{
  printf("%s %s\n", ok ? "PASS" : "FAIL", name);
  ok ? passed++ : failed++;
}

static wchar cell(int row, int col)
{
  return term.lines[row]->chars[col].chr;
}

static void feed(const char *s)
{
  term_write(s, strlen(s));
}

static void test_base64(void)
{
  char out[64];
  int n = base64_encode((const unsigned char *)"Man", 3, out, sizeof out);
  check("base64 encode Man", n == 4 && !strncmp(out, "TWFu", 4));
  n = base64_encode((const unsigned char *)"Ma", 2, out, sizeof out);
  check("base64 encode Ma pads once", n == 4 && !strncmp(out, "TWE=", 4));
  n = base64_encode((const unsigned char *)"M", 1, out, sizeof out);
  check("base64 encode M pads twice", n == 4 && !strncmp(out, "TQ==", 4));
  n = base64_decode("SGVsbG8gd29ybGQ=", 16, out, sizeof out);
  check("base64 decode Hello world", n == 11 && !memcmp(out, "Hello world", 11));
  n = base64_decode("SGV", 3, out, sizeof out);
  check("base64 decode rejects a short quantum", n == B64_INVALID_LEN);
  n = base64_decode("!!!!", 4, out, sizeof out);
  check("base64 decode rejects non-alphabet input", n < 0);
}

static void test_term(void)
{
  mt_init_term();
  term_resize(5, 20, false);
  feed("ab\033[2;3Hxyz\033[1;31mQ");
  check("plain text lands at 0,0", cell(0, 0) == 'a' && cell(0, 1) == 'b');
  check("CUP moves cursor", cell(1, 2) == 'x' && cell(1, 3) == 'y' && cell(1, 4) == 'z');
  check("text after CUP continues", cell(1, 5) == 'Q');
  check("SGR sets foreground colour",
        (term.lines[1]->chars[5].attr.attr & ATTR_FGMASK) != (ATTR_DEFFG & ATTR_FGMASK));
  check("SGR leaves earlier cells alone",
        (term.lines[1]->chars[4].attr.attr & ATTR_FGMASK) == (ATTR_DEFFG & ATTR_FGMASK));
  check("cursor column after text", term.curs.x == 6 && term.curs.y == 1);
  feed("\033[3;1H\303\251\r\nnext");
  check("UTF-8 decoded to one cell", cell(2, 0) == 0xE9);
  check("CR LF starts next line", cell(3, 0) == 'n' && cell(3, 3) == 't');
  feed("\033[2J");
  check("ED 2 clears the screen", cell(0, 0) == ' ' && cell(1, 2) == ' ' && cell(3, 0) == ' ');
  feed("\033[H12345\033[3D\033[1@Z");
  check("ICH shifts cells right", cell(0, 0) == '1' && cell(0, 1) == '2' && cell(0, 2) == 'Z' && cell(0, 3) == '3');
}

static void test_sixel(void)
{
  const char *img = "\"1;1;20;6#0;2;0;0;0#1;2;100;0;0#1!20~";
  sixel_state_t *st = calloc(1, sizeof *st);
  unsigned char *body = (unsigned char *)strdup(img);
  int ok = sixel_parser_init(st, 0xFFFFFF, 0, 0) >= 0 &&
           sixel_parser_parse(st, body, (int)strlen(img)) >= 0;
  unsigned char *pixels = ok ? sixel_parser_finalize(st) : NULL;
  check("sixel image parsed", pixels != NULL);
  check("sixel image is padded to 3x1 cells (24x16)", pixels && st->image.width == 24 && st->image.height == 16);
  check("sixel pixel is palette red (BGR order)", pixels && pixels[0] == 0 && pixels[1] == 0 && pixels[2] == 255);
  check("sixel run covers 20 columns, then background",
        pixels && pixels[19 * 4 + 2] == 255 && pixels[20 * 4 + 2] == 0 && pixels[5 * 24 * 4 + 2] == 255 &&
        pixels[6 * 24 * 4 + 2] == 0);
  sixel_parser_deinit(st);
  free(pixels);
  free(body);
  free(st);
}

static void test_bidi(void)
{
  bidi_char line[3] = {{0x5d0, 0x5d0, 0}, {0x5d1, 0x5d1, 1}, {0x5d2, 0x5d2, 2}};
  do_bidi(false, 0, false, false, line, 3);
  check("bidi reverses an RTL run", line[0].origwc == 0x5d2 && line[1].origwc == 0x5d1 && line[2].origwc == 0x5d0);
  bidi_char ltr[3] = {{'a', 'a', 0}, {'b', 'b', 1}, {'c', 'c', 2}};
  do_bidi(false, 0, false, false, ltr, 3);
  check("bidi keeps an LTR run", ltr[0].origwc == 'a' && ltr[1].origwc == 'b' && ltr[2].origwc == 'c');
}

int main(void)
{
  test_base64();
  test_term();
  test_sixel();
  test_bidi();
  printf("SUMMARY: %d passed, %d failed\n", passed, failed);
  return failed != 0;
}
