#ifndef SHA1_H_
#define SHA1_H_

#include <stddef.h>
#include "typedefs.h"

/***********
  MACROS
 **********/
#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

/***********
  STRUCTS
 **********/
typedef struct {
  u32 curr_hash[5];
  u8 buf[SHA1_BLOCK_SIZE];
  u64 total_len;
  size_t buf_len;
} sha1_context;

/*****************
  INLINE HELPERS
 ****************/

static inline u32 rotate32(u32 x, unsigned bits) {
  return ((x << bits) | (x >> (32 - bits)));
}
static inline u32 load_be32(const u8* x) {
  return (((u32)x[0] << 24) | (((u32)x[1] << 16)) | (((u32)x[2] << 8)) |
          ((u32)x[3]));
}
static inline void store_be32(u8* out, const u32* in) {
  out[0] = (u8)(*in >> 24);
  out[1] = (u8)(*in >> 16);
  out[2] = (u8)(*in >> 8);
  out[3] = (u8)(*in);
}
static inline size_t sha1_min(size_t block_size, size_t len) {
  return (block_size < len) ? block_size : len;
}

/********************
  PRIMARY FUNCTIONS
 *******************/

// set initial values in context
void sha1_init(sha1_context* context);
// compress data block, via main 80 iteration loop, modifying current hash
void sha1_compress_block(u32 h[5], const u8 block[SHA1_BLOCK_SIZE]);
// append data to context buffer, and compress if necessary
void sha1_update(sha1_context* context, const u8* data, size_t len);
// compress any remaining data in context buffer, write final digest in
// big-endian byte-order
void sha1_final(sha1_context* context, u8 out[SHA1_DIGEST_SIZE]);
// runs full SHA1 algorithm on msg and stores in out
void sha1_msg_to_hash(const char* msg, u8 out[SHA1_DIGEST_SIZE]);

#endif  // SHA1_H_
