#include "handler.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "SHA1.h"
#include "errno_print.h"
#include "utils.h"

/*************
 * CONSTANTS *
 ************/

// TODO accept a .dgitignore
static const char* ignore_dirent[] = {".",
                                      "..",
                                      ".dgit",
                                      ".git",
                                      ".gitignore",
                                      ".cache",
                                      "compile_commands.json"};
static size_t num_ignored_dirents =
    sizeof(ignore_dirent) / sizeof(ignore_dirent[0]);

/********************
 * HELPER FUNCTIONS *
 *******************/

static inline int entry_ignored(char* dirent) {
  for (size_t i = 0; i < num_ignored_dirents; ++i) {
    if (str_equal(ignore_dirent[i], dirent)) {
      return 1;
    }
  }
  return 0;
}
static void print_hex(const u8* buf, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    printf("%02x", buf[i]);
  }
  printf("\n");
}

int handle_init(void) {
  // create hidden directories, (mode 0777 -- rwx for everyone)
  if (mkdir(".dgit", 0777) == -1 && errno != EEXIST) {
    fprintf(stderr, "Failed to create directory\n");
    print_errno();
    return -1;
  }

  if (mkdir(".dgit/objects", 0777) == -1 && errno != EEXIST) {
    fprintf(stderr, "Failed to create directory\n");
    print_errno();
    return -1;
  }

  return 0;
}

int handle_hash_object(char* path, char* file_type, int print_hash) {
  int return_code = -1;

  int in_fd = -1;
  FILE* in = NULL;
  FILE* out = NULL;

  int tmp_fd = -1;
  int tmp_created = 0;
  char tmp_name[256];
  tmp_name[0] = '\0';

  in_fd = open(path, O_RDONLY);
  if (in_fd == -1) {
    fprintf(stderr, "Failed to open %s\n", path);
    print_errno();
    goto cleanup;
  }

  // get file size for SHA-1 header
  struct stat st;
  if (fstat(in_fd, &st) != 0) {
    fprintf(stderr, "Failed to parse %s size\n", path);
    print_errno();
    goto cleanup;
  }
  off_t f_size = st.st_size;

  in = fdopen(in_fd, "rb");
  if (in == NULL) {
    fprintf(stderr, "Failed to open file\n");
    print_errno();
    goto cleanup;
  }
  in_fd = -1;

  // creates a file with a unique temporary filename
  // last six letters of template must be "XXXXXX"
  char tmp_template[] = ".dgit/objects/tmpXXXXXX";
  tmp_fd = mkstemp(tmp_template);
  if (tmp_fd == -1) {
    fprintf(stderr, "Failed to create temp object file\n");
    print_errno();
    goto cleanup;
  }

  int temp_rename = snprintf(tmp_name, sizeof(tmp_name), "%s", tmp_template);
  if (temp_rename < 0 || (size_t)temp_rename >= sizeof(tmp_name)) {
    fprintf(stderr, "Temp path too long\n");
    goto cleanup;
  }
  tmp_created = 1;

  // write to temp file first, and rename at the end so code only reads input
  // file once
  out = fdopen(tmp_fd, "wb");
  if (out == NULL) {
    fprintf(stderr, "Failed to open temp file\n");
    print_errno();
    goto cleanup;
  }
  tmp_fd = -1;

  sha1_context context;
  sha1_init(&context);

  u8 buf[512];

  // construct header according to Git object header: <type> <size>\0
  char header[128];
  int header_len = snprintf(header, sizeof(header), "%s %lld%c", file_type,
                            (long long)f_size, '\0');
  if (header_len < 0 || (size_t)header_len >= sizeof(header)) {
    fprintf(stderr, "Failed to create header\n");
    goto cleanup;
  }

  // write and hash object header
  sha1_update(&context, (u8*)header, (size_t)header_len);
  if (fwrite(header, 1, (size_t)header_len, out) != (size_t)header_len) {
    fprintf(stderr, "Failed to write object header\n");
    print_errno();
    goto cleanup;
  }

  // read all of input file, update SHA-1 state and write to object file
  while (1) {
    size_t num_bytes = fread(buf, 1, sizeof(buf), in);
    if (num_bytes > 0) {
      sha1_update(&context, buf, num_bytes);
      if (fwrite(buf, 1, num_bytes, out) != num_bytes) {
        fprintf(stderr, "Failed to write object data\n");
        print_errno();
        goto cleanup;
      }
    }

    if (num_bytes < sizeof(buf)) {
      if (ferror(in)) {
        fprintf(stderr, "Failed to read input file\n");
        print_errno();
        goto cleanup;
      }

      break;  // read in EOF
    }
  }

  u8 digest[20];
  sha1_final(&context, digest);

  // write digest (in hex) to file name
  char hashed_file_name[41];
  for (size_t i = 0; i < 20; ++i) {
    snprintf(&hashed_file_name[i * 2], 3, "%02x", digest[i]);
  }
  hashed_file_name[40] = '\0';

  char new_file_name[128];
  int write_name = snprintf(new_file_name, sizeof(new_file_name),
                            ".dgit/objects/%s", hashed_file_name);

  // rename file from tmp to SHA-1 hash
  if (write_name < 0 || (size_t)write_name >= sizeof(new_file_name)) {
    fprintf(stderr, "Object path too long\n");
    goto cleanup;
  }

  if (fclose(out) != 0) {
    out = NULL;
    fprintf(stderr, "Failed to close temp object file\n");
    print_errno();
    goto cleanup;
  }
  out = NULL;

  if (fclose(in) != 0) {
    in = NULL;
    fprintf(stderr, "Failed to close input file\n");
    print_errno();
    goto cleanup;
  }
  in = NULL;

  // if object already exists, return true and exit
  if (access(new_file_name, F_OK) == 0) {
    if (tmp_created) {
      remove(tmp_name);
      tmp_created = 0;
    }
    if (print_hash) {
      print_hex(digest, 20);
    }

    return_code = 0;
    goto cleanup;
  }

  if (rename(tmp_name, new_file_name) == -1) {
    fprintf(stderr, "Failed to rename %s to %s\n", tmp_name, new_file_name);
    print_errno();
    goto cleanup;
  }
  tmp_created = 0;  // rename succeeded

  if (print_hash) {
    print_hex(digest, 20);
  }

  return_code = 0;

cleanup:
  if (out) {
    fclose(out);
  }

  if (in) {
    fclose(in);
  }

  if (tmp_fd != -1) {
    close(tmp_fd);
  }

  if (in_fd != -1) {
    close(in_fd);
  }

  if (tmp_created && tmp_name[0] != '\0') {
    remove(tmp_name);
  }

  return return_code;
}

int handle_cat_file(char* oid) {
  int return_code = -1;
  FILE* file = NULL;

  char file_name[128];
  int write_name =
      snprintf(file_name, sizeof(file_name), ".dgit/objects/%s", oid);
  if (write_name < 0 || (size_t)write_name >= sizeof(file_name)) {
    fprintf(stderr, "Object path too long\n");
    goto cleanup;
  }

  file = fopen(file_name, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open %s\n", file_name);
    print_errno();
    goto cleanup;
  }

  // parse object header type from within "<type> <size>\0"
  char file_type[32];
  size_t idx = 0;

  while (1) {
    int c = fgetc(file);
    if (c == EOF) {
      fprintf(stderr, "Unexpected EOF in object header\n");
      goto cleanup;
    }

    if (c == ' ') {
      break;
    }

    if (idx + 1 >= sizeof(file_type)) {
      fprintf(stderr, "File type too large\n");
      goto cleanup;
    }

    file_type[idx++] = (char)c;
  }

  file_type[idx] = '\0';

  // TODO support more types
  if (!str_equal(file_type, "blob")) {
    fprintf(stderr, "File type '%s' not supported\n", file_type);
    goto cleanup;
  }

  // skip size field up to null terminator
  while (1) {
    int c = fgetc(file);
    if (c == EOF) {
      fprintf(stderr, "Unexpected EOF in object header\n");
      goto cleanup;
    }

    if (c == '\0') {
      break;
    }
  }

  char buf[2048];
  size_t bytes_read;

  while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
    if (fwrite(buf, 1, bytes_read, stdout) != bytes_read) {
      fprintf(stderr, "Error writing object to stdout\n");
      print_errno();
      goto cleanup;
    }
  }

  if (ferror(file)) {
    fprintf(stderr, "Error reading object file\n");
    print_errno();
    goto cleanup;
  }

  return_code = 0;

cleanup:
  if (file) {
    fclose(file);
  }

  return return_code;
}

int handle_write_tree(char* directory) {
  int return_code = -1;

  DIR* d = NULL;

  if (strlen(directory) >= 128) {
    fprintf(stderr, "Directory name too large %s\n", directory);
    goto cleanup;
  }

  d = opendir(directory);
  if (d == NULL) {
    fprintf(stderr, "Could not open directory %s\n", directory);
    print_errno();
    goto cleanup;
  }

  struct dirent* dir;
  while ((dir = readdir(d)) != NULL) {
    char* name = dir->d_name;

    if (entry_ignored(name)) {
      continue;
    }

    printf("Name %s. ", dir->d_name);
    if (dir->d_type == DT_DIR) {
      printf("Type: Directory\n");
      char new_dir[264];
      snprintf(new_dir, sizeof(new_dir), "%s/%s", directory, name);
      // TODO use SHA1_msg to create new directory (name is composed of all
      // files within tree (type, oid, name) then create directory and pass that
      // into the handle_write_tree
      handle_write_tree(new_dir);
    } else if (dir->d_type == DT_REG) {
      printf("Type: File\n");
      char file[264];
      snprintf(file, sizeof(file), "%s/%s", directory, name);

      for (size_t i = 0; i < num_ignored_dirents; ++i) {
        if (str_equal(name, ignore_dirent[i])) {
          continue;
        }
      }

      if (handle_hash_object(file, "blob", 0) == -1) {
        fprintf(stderr, "handle_hash_object failed\n");
        goto cleanup;
      }
    }
  }

  return_code = 0;

cleanup:
  if (d != NULL) {
    closedir(d);
  }

  return return_code;
}
