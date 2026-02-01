#include "errno_print.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void print_errno(void) {
  fprintf(stderr, "Error: %s\n", strerror(errno));

  return;
}
