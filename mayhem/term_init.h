// Headless terminal setup shared by the fuzz harness and the known-answer selftest.
#include "term.h"

static void mt_init_term(void)
{
#include "cfg_strings.h"
  cfg.font.name = u"";
  for (int i = 0; i < 12; i++)
    cfg.fontfams[i].name = u"";
  cfg.term = "xterm";
  cfg.sixel_clip_char = u" ";
  cfg.allow_blinking = true;
  cfg.bidi = 2;
  cfg.cursor_type = 0;
  cfg.scrollback_lines = 100;
  cfg.max_scrollback_lines = 200;
  cfg.printable_controls = 0;
  cfg.charwidth = 0;
  cfg.old_bold = false;
  cfg.bell_type = 0;
  cfg.enable_remap_ctrls = false;
  cfg.tek_glow = false;
  cfg.input_clears_selection = true;
  term_reset(true);
  term_resize(24, 80, false);
}
