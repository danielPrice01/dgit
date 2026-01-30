#include "SHA1.h"
#include <string.h>

void sha1_init(sha1_context* context) {
  // set starting H0-H4 to standard SHA-1 starting hash (FIPS 180-4)
  context->curr_hash[0] = 0x67452301U;
  context->curr_hash[1] = 0xEFCDAB89U;
  context->curr_hash[2] = 0x98BADCFEU;
  context->curr_hash[3] = 0x10325476U;
  context->curr_hash[4] = 0xC3D2E1F0U;

  context->total_len = 0;
  context->buf_len = 0;

  memset(context->buf, 0, SHA1_BLOCK_SIZE);  // zero out buffer
}

void sha1_update(sha1_context* context, const u8* data, size_t len) {
  context->total_len += len;

  if (context->buf_len > 0) {
    // copy data into remaining space in context buffer
    size_t buf_append_size = sha1_min(SHA1_BLOCK_SIZE - context->buf_len, len);
    memcpy(context->buf + context->buf_len, data, buf_append_size);

    data = data + buf_append_size;
    context->buf_len += buf_append_size;
    len -= buf_append_size;

    // if context buffer is full (guaranteed to happen if len > empty buffer
    // space), compress it and "clear" it
    if (context->buf_len == SHA1_BLOCK_SIZE) {
      sha1_compress_block(context->curr_hash, context->buf);
      context->buf_len = 0;
    }
  }

  // compress 512 bit blocks of data until <512 bits left
  while (len >= SHA1_BLOCK_SIZE) {
    sha1_compress_block(context->curr_hash, data);
    len -= SHA1_BLOCK_SIZE;
    data += SHA1_BLOCK_SIZE;
  }

  // store any remaining data in buffer to be compressed on next call
  if (len > 0) {
    memcpy(context->buf, data, len);
    context->buf_len = len;
  }
}

void sha1_compress_block(u32 h[5], const u8 block[SHA1_BLOCK_SIZE]) {
  // initialize 80 words for this compression
  u32 W[80];

  for (size_t i = 0; i < 16; ++i) {
    W[i] = load_be32(block + (4 * i));
  }

  for (size_t i = 16; i < 80; ++i) {
    W[i] = rotate32(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
  }

  // save local copies of H0-H4
  u32 a = h[0];
  u32 b = h[1];
  u32 c = h[2];
  u32 d = h[3];
  u32 e = h[4];

  u32 f, k;

  // the main 80-iter loop of SHA1 compression, follows FIPS 180-4
  for (size_t i = 0; i < 80; ++i) {
    if (i <= 19) {
      f = (b & c) | ((~b) & d);
      k = 0x5A827999U;  // floor(2^30 * sqrt(2))
    } else if (i <= 39) {
      f = b ^ c ^ d;
      k = 0x6ED9EBA1U;  // floor(2^30 * sqrt(3))
    } else if (i <= 59) {
      f = (b & c) | (b & d) | (c & d);
      k = 0x8F1BBCDCU;  // floor(2^30 * sqrt(5))
    } else {
      f = b ^ c ^ d;
      k = 0xCA62C1D6U;  // floor(2^30 * sqrt(10))
    }

    u32 tmp = rotate32(a, 5) + e + f + k + W[i];

    e = d;
    d = c;
    c = rotate32(b, 30);
    b = a;
    a = tmp;
  }

  // add the compressed chunk's state back onto running hash value
  h[0] += a;
  h[1] += b;
  h[2] += c;
  h[3] += d;
  h[4] += e;
}

void sha1_final(sha1_context* context, u8 out[SHA1_DIGEST_SIZE]) {
  // convert length of total message passed to SHA1 algo into corresponding
  // 8-byte value
  u64 bitlen = context->total_len * 8;

  // append SHA-1 padding, 1 bit followed by 0s
  context->buf[context->buf_len++] = 0x80;

  // guarantee there is enough bytes left to add message size
  if (context->buf_len > 56) {
    memset(context->buf + context->buf_len, 0,
           SHA1_BLOCK_SIZE - context->buf_len);
    sha1_compress_block(context->curr_hash, context->buf);
    context->buf_len = 0;
  }

  // add message size in big endian and compress
  memset(context->buf + context->buf_len, 0, 56 - context->buf_len);
  context->buf_len = 56;

  context->buf[56] = (u8)(bitlen >> 56);
  context->buf[57] = (u8)(bitlen >> 48);
  context->buf[58] = (u8)(bitlen >> 40);
  context->buf[59] = (u8)(bitlen >> 32);
  context->buf[60] = (u8)(bitlen >> 24);
  context->buf[61] = (u8)(bitlen >> 16);
  context->buf[62] = (u8)(bitlen >> 8);
  context->buf[63] = (u8)(bitlen);

  sha1_compress_block(context->curr_hash, context->buf);

  // output final hash in big-endian byte-order
  for (size_t i = 0; i < 5; ++i) {
    store_be32(out + 4 * i, &context->curr_hash[i]);
  }
}

// helper function, mostly for testing
void sha1_msg_to_hash(const char* msg, u8 out[SHA1_DIGEST_SIZE]) {
  sha1_context context;
  sha1_init(&context);
  sha1_update(&context, (u8*)msg, strlen(msg));
  sha1_final(&context, out);
}
