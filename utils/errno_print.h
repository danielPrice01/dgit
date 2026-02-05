#ifndef ERRNO_PRINT_H_
#define ERRNO_PRINT_H_

#include <errno.h>
#include <stdio.h>
#include <string.h>

static inline void print_errno(void) {
  fprintf(stderr, "Error: %s\n", strerror(errno));

  return;
}
static inline void print_errno(void);

#endif  // ERRNO_PRINT_H_
