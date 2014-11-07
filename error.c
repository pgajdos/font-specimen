#include "error.h"

#include <stdio.h>

int debug = 0;

void error(const char *string)
{
  if (debug)
    fprintf(stderr, "%s\n", string);
}
