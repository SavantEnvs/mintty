// Feeds arbitrary bytes to mintty's terminal escape-sequence parser (term_write), headless.
#include "term_init.h"
#include "winpriv.h"
#include "charset.h"
#include <stdint.h>
#include <string.h>

void mt_reset_term_state(void);
void mt_reset_termout_state(void);

static bool inited;
static config cfg0;
static COLORREF colours0[COLOUR_NUM];

// term_reset() does not restore these: restore them so each input's result depends only on that input.
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (!inited) {
    mt_init_term();
    memcpy(&cfg0, &cfg, sizeof cfg);
    memcpy(colours0, colours, sizeof colours);
    inited = true;
  }
  else {
    memcpy(&cfg, &cfg0, sizeof cfg);
    memcpy(colours, colours0, sizeof colours);
    term_reset(true);
  }
  lines_scrolled = 0;  // the real front end zeroes it on every paint (wintext.c)
  cs_mb1towc(0, 0);    // the real decoder's state is cleared on reset (charset.c)
  mt_reset_term_state();     // term.c's hyperlink table (links, nlinks, linkid)
  mt_reset_termout_state();  // termout.c's file-statics (REP char, mode/attr/colour stacks) and the Tek globals tek_mode/tek_bypass
  term_write((const char *)data, size);
  return 0;
}
