#ifndef CLI_H_
#define CLI_H_

#include <stdio.h>
#include "handler.h"

typedef int (*cmd_fn)(int argc, char** argv);

static int cmd_init(int argc, char** argv) {
  (void)argc;
  (void)argv;
  return handle_init();
}

static int cmd_hash_object(int argc, char** argv) {
  (void)argc;
  return handle_hash_object(argv[2], "blob", 1);
}

static int cmd_cat_file(int argc, char** argv) {
  (void)argc;
  return handle_cat_file(argv[2]);
}

static int cmd_write_tree(int argc, char** argv) {
  (void)argc;
  return handle_write_tree(argv[2]);
}

typedef struct {
  const char* name;
  int argc_required;
  const char* usage;
  cmd_fn fn;
} command;

static const command commands[] = {
    {"init", 2, "init", cmd_init},
    {"hash-object", 3, "hash-object <path>", cmd_hash_object},
    {"cat-file", 3, "cat-file <oid>", cmd_cat_file},
    {"write-tree", 3, "write-tree <directory>", cmd_write_tree},
};

static const size_t commands_count = sizeof(commands) / sizeof(commands[0]);

static void print_usage(const char* prog) {
  fprintf(stderr, "Usage: %s <command> [args]\n", prog);
  for (size_t i = 0; i < commands_count; ++i) {
    fprintf(stderr, "  %s %s\n", prog, commands[i].usage);
  }
}

static int cli_run(int argc, char** argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  // iterate through all available commands and check if input matches any
  for (size_t i = 0; i < commands_count; ++i) {
    if (str_equal(argv[1], commands[i].name)) {
      if (argc != commands[i].argc_required) {
        fprintf(stderr, "usage: %s %s\n", argv[0], commands[i].usage);
        return 1;
      }

      if (commands[i].fn(argc, argv) == -1) {
        fprintf(stderr, "%s failed\n", commands[i].name);
        return 1;
      }

      return 0;
    }
  }

  fprintf(stderr, "%s: unrecognized command '%s'\n", argv[0], argv[1]);
  print_usage(argv[0]);
  return 0;
}

#endif  // CLI_H_
