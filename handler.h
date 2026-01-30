#ifndef HANDLER_H_
#define HANDLER_H_

#include <stdio.h>
#include "typedefs.h"
#include "utils.h"

/************
  CONSTANTS
 ***********/

static const char* ignore_dirent[] = {".", "..", ".dgit"};
static size_t num_ignored_dirents =
    sizeof(ignore_dirent) / sizeof(ignore_dirent[0]);

/***************************
  PRIMARY FUNCTION HANDLERS
 **************************/

// return -1 on failure, 0 on success
int handle_init(void);
// note: this does not compress files or distribute across many directories for
// sake of simplicity
int handle_hash_object(char* path, char* file_type, int print_hash);
int handle_cat_file(char* path);
int handle_write_tree(char* directory);

#endif  // HANDLER_H_
