// Builds src/term.c unmodified and adds a reset for its hyperlink table, which term_reset() leaves alone.
#include "../src/term.c"

void mt_reset_term_state(void)
{
  for (int i = 0; i < nlinks; i++)
    free(links[i]);
  free(links);
  links = 0;
  nlinks = 0;
  linkid = 0;
}
