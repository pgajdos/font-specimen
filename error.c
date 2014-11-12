#include "error.h"

#include <stdio.h>

int debug = 0;

void set_debug(int on)
{
  debug = on;
}

void font_specimen_error(const char *string)
{
  if (debug)
    fprintf(stderr, "%s\n", string);
}
