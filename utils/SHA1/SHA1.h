#ifndef SHA1_H_
#define SHA1_H_

#include <stddef.h>
#include "typedefs.h"

#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

typedef struct {
        u32 curr_hash[5];
        u8 buf[SHA1_BLOCK_SIZE];
        u64 total_len;
        size_t buf_len;
} sha1_context;

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
