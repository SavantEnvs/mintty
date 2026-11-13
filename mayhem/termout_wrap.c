// Builds src/termout.c unmodified and adds a reset for its file-static state and the Tek globals, which term_reset() leaves alone.
#include "../src/termout.c"

void mt_reset_termout_state(void)
{
  tek_mode = TEKMODE_OFF;  // real tek_reset()/tek_page() do this; the harness stubs them
  tek_bypass = false;
  prev_state = 0;
  last_high = 0;
  last_char = 0;
  last_width = 0;
  last_attr = (cattr){.attr = ATTR_DEFAULT, .truefg = 0, .truebg = 0, .ulcolr = (colour)-1};
  esc_mod0 = 0;
  esc_mod1 = 0;
  cattr_stack_len = 0;
  free(mode_stack);
  mode_stack = 0;
  mode_stack_len = 0;
  for (int i = 0; i < 10; i++) {
    free(colours_stack[i]);
    colours_stack[i] = 0;
  }
  colours_cur = 0;
  colours_num = 0;
}
