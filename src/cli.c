#include <stdio.h>
#include <sys/types.h>
#include "handler.h"
#include "utils.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage %s <command>\n", argv[0]);
    return 1;
  }

  if (str_equal(argv[1], "init")) {
    if (handle_init() == -1) {
      fprintf(stderr, "init failed\n");
      return 1;
    }
  } else if (str_equal(argv[1], "hash-object")) {
    if (handle_hash_object(argv[2], "blob", 1) == -1) {
      fprintf(stderr, "hash-object failed\n");
      return 1;
    }
  } else if (str_equal(argv[1], "cat-file")) {
    if (handle_cat_file(argv[2]) == -1) {
      fprintf(stderr, "cat-file failed\n");
      return 1;
    }
  } else if (str_equal(argv[1], "write-tree")) {
    if (handle_write_tree(argv[2]) == -1) {
      fprintf(stderr, "write-tree failed\n");
      return 1;
    }
  } else {
    printf("%s unrecognized command\n", argv[1]);
  }

  return 0;
}
