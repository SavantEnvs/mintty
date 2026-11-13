/* Link-time stand-ins for the Windows/GUI half of mintty, so its terminal core (term*.c) runs headless. */
#include "term.h"
#include "win.h"
#include "winpriv.h"
#include "winimg.h"
#include "child.h"
#include "charset.h"
#include "print.h"
#include "tek.h"
#include "winsearch.h"
#include <termios.h>

config cfg, new_cfg, file_cfg;
COLORREF colours[COLOUR_NUM];
int PADDING, OFFSET, font_size = 9, cell_width = 8, cell_height = 16, line_scale = 100, lines_scrolled;
bool force_imgs, tek_bypass, cs_ambig_wide, cs_single_forced;
enum tekmode tek_mode;
char *home = "";

#define NOP(ret, name, args) ret name args { return (ret)0; }
#define VNOP(name, args) void name args { }

void child_printf(const char *fmt, ...) { (void)fmt; }
VNOP(child_resize, (struct winsize *w))
VNOP(child_send, (const char *s, uint n))
VNOP(child_sendw, (const wchar *s, uint n))
VNOP(child_set_fork_dir, (char *d))
VNOP(child_write, (const char *s, uint n))
uchar *child_termios_chars(void) { static uchar t[NCCS]; return t; }

char *asform(const char *fmt, ...) { (void)fmt; return strdup(""); }
void wstrset(wstring *sp, wstring s) { (void)sp; (void)s; }

NOP(bool, dw_check_glyphs, (xchar *x, uint n, cattrflags a))
NOP(bool, dw_has_glyph, (xchar x, cattrflags a))
NOP(bool, get_scroll_lock, (void))
NOP(bool, parse_colour, (string s, colour *c))
NOP(bool, win_confirm_text, (wchar *t, wchar *c))
NOP(bool, win_get_ime, (void))
NOP(bool, winimg_new, (imglist **p, char *id, unsigned char *px, uint len, int l, int t, int w, int h, int pw, int ph, bool ar, int cx, int cy, int cw, int ch, int attr))
NOP(bool, win_is_iconic, (void))
cattr apply_attr_colour(cattr a, attr_colour_mode m) { (void)m; return a; }
NOP(char *, get_clipboard, (void))
NOP(char *, get_resource_file, (wstring s, wstring r, bool w))
NOP(char *, guardpath, (string p, int l))
char *save_filename(char *s) { (void)s; return strdup("/dev/null"); }
NOP(char *, win_get_title, (void))
colour brighten(colour c, colour a, bool m) { (void)a; (void)m; return c; }
colour win_get_colour(colour_i i) { return colours[i]; }
int cursor_blink_ticks(void) { return 500; }
int get_tick_count(void) { return 0; }
NOP(int, search_monitors, (int *x, int *y, HMONITOR m, int g, MONITORINFO *mi))
int termattrs_equal_fg(cattr *a, cattr *b) { return a->truefg == b->truefg && a->attr == b->attr; }
int win_char_width(xchar c, cattrflags a) { (void)a; return xcwidth(c); }
uint colour_dist(colour a, colour b) { return a == b ? 0 : 1; }
uint win_get_font_size(void) { return font_size; }
VNOP(compose_clear, (void))
VNOP(do_update, (void))
VNOP(printer_finish_job, (void))
VNOP(printer_start_job, (wstring n))
VNOP(printer_write, (char *s, uint n))
VNOP(printer_wwrite, (wchar *s, uint n))
NOP(wstring, printer_get_default, (void))
VNOP(scale_to_image_ratio, (void))
VNOP(set_arg_option, (string n, string v))
VNOP(set_cursor_style, (int a, wstring s))
VNOP(sync_scroll_lock, (bool l))
VNOP(taskbar_progress, (int p))
VNOP(toggle_status_line, (void))
VNOP(term_save_image, (bool o))
VNOP(winimgs_clear, (void))
VNOP(winimg_destroy, (imglist *i))
VNOP(winimg_lazyinit, (imglist *i))
VNOP(win_beep, (uint t, float v, float f, uint ms))
VNOP(win_bell, (config *c))
VNOP(win_capture_mouse, (void))
VNOP(win_change_font, (uint i, wstring f))
VNOP(win_check_glyphs, (wchar *w, uint n, cattrflags a))
VNOP(win_copy_as, (const wchar *d, cattr *c, int l, char w))
VNOP(win_copy, (const wchar *d, cattr *c, int l))
VNOP(win_copy_text, (const char *s))
VNOP(win_copy_title, (void))
VNOP(win_emoji_show, (int x, int y, wchar *e, void **b, int *bl, int el, ushort la, bool it))
VNOP(win_get_locator_info, (int *x, int *y, int *b, bool p))
VNOP(win_get_pixels, (int *h, int *w, bool b))
VNOP(win_get_screen_chars, (int *r, int *c))
VNOP(win_get_scrpos, (int *x, int *y, bool b))
VNOP(win_invalidate_all, (bool c))
VNOP(win_led, (int l, bool s))
VNOP(win_margin_bell, (config *c))
VNOP(win_maximise, (int m))
VNOP(win_open, (wstring p, bool a))
VNOP(win_paste, (void))
VNOP(win_popup_menu, (mod_keys m))
VNOP(win_reset_colours, (void))
VNOP(win_restore_title, (void))
VNOP(win_save_title, (void))
VNOP(win_schedule_update, (void))
VNOP(win_set_chars, (int r, int c))
void win_set_colour(colour_i i, colour c) { colours[i] = c; }
VNOP(win_set_font_size, (int s, bool b))
VNOP(win_set_geom, (int y, int x, int h, int w))
VNOP(win_set_icon, (char *s, int i))
VNOP(win_set_iconic, (bool b))
VNOP(win_set_ime, (bool n))
VNOP(win_set_pixels, (int h, int w))
VNOP(win_set_pos, (int x, int y))
VNOP(win_set_scrollview, (int p, int l, int h))
VNOP(win_set_timer, (void_fn cb, uint t))
VNOP(win_set_title, (char *t))
VNOP(win_set_zorder, (bool t))
VNOP(win_sound, (char *s, uint o))
VNOP(win_text, (int x, int y, wchar *t, int l, cattr a, cattr *ta, ushort la, char r, char s, bool c, uchar p))
VNOP(win_update, (bool s))
VNOP(win_update_mouse, (void))
VNOP(win_update_scrollbar, (bool i))
VNOP(win_zoom_font, (int z, bool s))
wchar win_combine_chars(wchar bc, wchar cc, cattrflags a) { (void)bc; (void)cc; (void)a; return 0; }
wchar win_linedraw_char(int i) { return 0x2500 + i; }
NOP(wstring, win_get_font, (uint i))
NOP(wchar *, char_code_indication, (uint *w))
wchar get_errch(wchar *w, cattrflags a) { (void)w; (void)a; return 0xFFFD; }
NOP(wchar *, path_posix_to_win_w, (const char *p))

VNOP(tek_address, (char *a))
VNOP(tek_alt, (bool a))
VNOP(tek_beam, (bool d, bool w, char v))
VNOP(tek_clear, (void))
VNOP(tek_enq, (void))
VNOP(tek_font, (short f))
VNOP(tek_gin, (void))
VNOP(tek_init, (bool r, int g))
VNOP(tek_intensity, (bool d, int i))
VNOP(tek_move_by, (int dy, int dx))
VNOP(tek_pen, (bool on))
VNOP(tek_send_address, (void))
VNOP(tek_set_font, (wchar *f))
VNOP(tek_step, (char c))
VNOP(tek_write, (wchar c, int w))
VNOP(tek_page, (void))
VNOP(tek_paint, (void))
VNOP(tek_reset, (void))
VNOP(tek_copy, (wchar *fn))

/* charset: UTF-8 only */
string cs_get_locale(void) { return "C.UTF-8"; }
VNOP(cs_set_locale, (string l))
VNOP(cs_set_mode, (cs_mode m))
/* Same incremental UTF-8 decoding as charset.c's UTF-8 branch (utfrtowc), which cs_mb1towc uses. */
int cs_mb1towc(wchar *pwc, char ch)
{
  static int buflen, utflen;
  static unsigned int val;
  unsigned char c = ch;

  if (!pwc) {
    buflen = utflen = 0;
    return 0;
  }
  if (!(c & 0x80)) {
    *pwc = c;
    buflen = 0;
    return 1;
  }
  if ((c & 0xC0) == 0x80) {
    if (!utflen)
      return -1;
    buflen++;
    val = (val << 6) | (c & 0x3F);
    if (buflen < utflen) {
      if (buflen == 3) {
        val -= 0x10000 >> 6;
        *pwc = 0xD800 | (val >> 4);
        val = (val & (0x03FF >> 6)) | (0xDC00 >> 6);
        return 1;
      }
      return -2;
    }
    *pwc = val;
    buflen = utflen = 0;
    return 1;
  }
  if ((c & 0xE0) == 0xC0) { utflen = 2; val = c & 0x1F; buflen = 1; return -2; }
  if ((c & 0xF0) == 0xE0) { utflen = 3; val = c & 0x0F; buflen = 1; return -2; }
  if ((c & 0xF8) == 0xF0) { utflen = 4; val = c & 0x07; buflen = 1; return -2; }
  buflen = utflen = 0;
  return -1;
}
wchar cs_btowc_glyph(char c) { return (uchar)c; }
wchar *cs__mbstowcs(const char *s) { size_t n = strlen(s) + 1; wchar *w = calloc(n + 1, sizeof *w); mbstowcs(w, s, n); return w; }
wchar *cs__utftowcs(const char *s) { return cs__mbstowcs(s); }
char *cs__wcstoutf(const wchar *ws)
{
  size_t n = wcslen(ws), o = 0;
  char *s = malloc(n * 4 + 1);
  for (size_t i = 0; i < n; i++) {
    xchar c = ws[i];
    if (is_high_surrogate(c) && i + 1 < n && is_low_surrogate(ws[i + 1]))
      c = combine_surrogates(c, ws[++i]);
    if (c < 0x80) s[o++] = c;
    else if (c < 0x800) { s[o++] = 0xC0 | c >> 6; s[o++] = 0x80 | (c & 0x3F); }
    else if (c < 0x10000) { s[o++] = 0xE0 | c >> 12; s[o++] = 0x80 | (c >> 6 & 0x3F); s[o++] = 0x80 | (c & 0x3F); }
    else { s[o++] = 0xF0 | c >> 18; s[o++] = 0x80 | (c >> 12 & 0x3F); s[o++] = 0x80 | (c >> 6 & 0x3F); s[o++] = 0x80 | (c & 0x3F); }
  }
  s[o] = 0;
  return s;
}
char *cs__wcstombs(const wchar *ws) { return cs__wcstoutf(ws); }
