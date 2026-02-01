#include "handler.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "SHA1.h"
#include "errno_print.h"
#include "utils.h"

/*************
 * CONSTANTS *
 ************/

static const char* ignore_dirent[] = {".", "..", ".dgit"};
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
  if (mkdir(".dgit", 0777)) {
    fprintf(stderr, "Failed to create directory\n");
    print_errno();
    return -1;
  }

  if (mkdir(".dgit/objects", 0777)) {
    fprintf(stderr, "Failed to create directory\n");
    return -1;
  }

  return 0;
}

int handle_hash_object(char* path, char* file_type, int print_hash) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Failed to open %s\n", path);
    return -1;
  }

  // get file size for SHA-1 header
  struct stat st;
  off_t f_size;
  if (fstat(fd, &st) != 0) {
    fprintf(stderr, "Failed to parse %s size\n", path);
    close(fd);
    return -1;
  }
  f_size = st.st_size;

  FILE* in = fdopen(fd, "rb");
  if (in == NULL) {
    fprintf(stderr, "Failed to open file\n");
    return -1;
  }

  // write to temp file first, and rename at the end so code only reads input
  // file once
  FILE* out = fopen("./.dgit/objects/tmp", "wb");
  if (out == NULL) {
    fprintf(stderr, "Failed to create file\n");
    fclose(in);
    return -1;
  }

  sha1_context context;
  sha1_init(&context);

  u8 buf[512];

  // construct header according to Git object header: <type> <size>\0
  char header[128];
  int header_len = snprintf(header, sizeof(header), "%s %lld%c", file_type,
                            (long long)f_size, '\0');
  if (header_len < 0 || (size_t)header_len >= sizeof(header)) {
    fprintf(stderr, "Failed to create prefix\n");
    fclose(in);
    fclose(out);
    remove("./.dgit/objects/tmp");
    return -1;
  }

  // write and hash object header
  sha1_update(&context, (u8*)header, (size_t)header_len);
  if (fwrite(header, 1, (size_t)header_len, out) != (size_t)header_len) {
    fprintf(stderr, "Failed to write object header\n");
    fclose(in);
    fclose(out);
    remove("./.dgit/objects/tmp");
    return -1;
  }

  size_t num_bytes;

  // read all of input file, update SHA-1 state and write to object file
  while ((num_bytes = fread(buf, 1, sizeof(buf), in)) > 0) {
    sha1_update(&context, buf, num_bytes);

    if (fwrite(buf, 1, num_bytes, out) != num_bytes) {
      fprintf(stderr, "Failed to write object data\n");
      fclose(in);
      fclose(out);
      remove("./.dgit/objects/tmp");
      return -1;
    }
  }

  if (ferror(in)) {
    fprintf(stderr, "Failed to read input file\n");
    fclose(in);
    fclose(out);
    remove("./.dgit/objects/tmp");
    return -1;
  }

  u8 digest[20];
  sha1_final(&context, digest);

  // write digest (in hex) to file name
  char hashed_file_name[41];
  for (size_t i = 0; i < 20; ++i) {
    snprintf(&hashed_file_name[i * 2], 3, "%02x", digest[i]);
  }

  hashed_file_name[40] = '\0';

  char new_file_name[128] = {".dgit/objects/"};
  snprintf(new_file_name, sizeof(new_file_name), ".dgit/objects/%s",
           hashed_file_name);

  fclose(in);
  fclose(out);

  // rename file from tmp to SHA-1 hash
  if (rename(".dgit/objects/tmp", new_file_name) == -1) {
    fprintf(stderr, "Failed to rename %s\n", ".dgit/objects/tmp");
    if (remove(".dgit/objects/tmp") == -1) {
      fprintf(stderr, "Failed to delete %s\n", ".dgit/objects/tmp");
    }
    return -1;
  }

  if (print_hash) {
    print_hex(digest, 20);
  }

  return 0;
}

int handle_cat_file(char* path) {
  char file_name[128];
  snprintf(file_name, sizeof(file_name), ".dgit/objects/%s", path);

  FILE* file = fopen(file_name, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open %s\n", file_name);
    return -1;
  }

  // parse object header type from within "<type> <size>\0"
  char file_type[32];
  int c;
  int idx = 0;

  while ((c = fgetc(file)) != EOF) {
    if (c == ' ') {
      break;
    }

    if ((size_t)(idx + 1) >= sizeof(file_type)) {
      fprintf(stderr, "File type too large\n");
      fclose(file);
      return -1;
    }

    file_type[idx++] = (char)c;
  }

  if (c == EOF) {
    fprintf(stderr, "Invalid file type");
    fclose(file);
    return -1;
  }

  file_type[idx] = '\0';

  // TODO support more types
  if (!str_equal(file_type, "blob")) {
    fprintf(stderr, "File type '%s' not supported\n", file_type);
    fclose(file);
    return -1;
  }

  int found = 0;

  // move file offset forwards to beginning of file content
  while ((c = fgetc(file)) != EOF) {
    if (c == '\0') {
      found = 1;
      break;
    }
  }

  if (!found) {
    fprintf(stderr, "Header not found\n");
    fclose(file);
    return -1;
  }

  if (ferror(file)) {
    fprintf(stderr, "Error reading file\n");
    fclose(file);
    return -1;
  }

  char buf[2048];
  size_t bytes_read;

  while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
    fwrite(buf, 1, bytes_read, stdout);
  }

  fclose(file);
  return 0;
}

int handle_write_tree(char* directory) {
  if (strlen(directory) >= 128) {
    fprintf(stderr, "Directory name too large %s\n", directory);
    return -1;
  }

  DIR* d;
  struct dirent* dir;

  d = opendir(directory);
  if (d == NULL) {
    fprintf(stderr, "Could not open directory %s\n", directory);
    return -1;
  }

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

      handle_hash_object(file, "blob", 0);
    }
  }

  return 0;
}
