#ifndef HANDLER_H_
#define HANDLER_H_

int handle_init(void);  // return -1 on failure, 0 on success
// note: this does not compress files or distribute across many directories for
// sake of simplicity
int handle_hash_object(char* path, char* file_type, int print_hash);
int handle_cat_file(char* path);
int handle_write_tree(char* directory);

#endif  // HANDLER_H_
