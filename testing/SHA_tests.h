#include <assert.h>
#include <string.h>
#include "SHA1.h"
#include "dtest.h"

static inline void hash(const char* msg, char* hex) {
        u8 out[SHA1_DIGEST_SIZE];
        sha1_msg_to_hash(msg, out);
        for (int i = 0; i < 20; i++) {
                sprintf(hex + i * 2, "%02x", out[i]);
        }
        hex[40] = '\0';
}

static inline void assert_hash_eq(const char* msg, const char* expected_hex) {
        u8 out[SHA1_DIGEST_SIZE];
        sha1_msg_to_hash(msg, out);
        char hex[41];
        for (int i = 0; i < 20; i++) {
                sprintf(hex + i * 2, "%02x", out[i]);
        }
        hex[40] = '\0';
        assert(strcmp(hex, expected_hex) == 0);
}

void test_sha1_empty(void) {
        char hex[41];
        hash("", hex);
        ASSERT_STR_EQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", hex);
}

void test_sha1_abc(void) {
        char hex[41];
        hash("abc", hex);
        ASSERT_STR_EQ("a9993e364706816aba3e25717850c26c9cd0d89d", hex);
}

void test_sha1_hello_world(void) {
        char hex[41];
        hash("hello world", hex);
        ASSERT_STR_EQ("2aae6c35c94fcfb415dbe95f408b9ce91ee846ed", hex);
}

void test_sha1_56_bytes(void) {
        char hex[41];
        hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", hex);
        // exactly the boundary where padding needs an extra block
        ASSERT_STR_EQ("84983e441c3bd26ebaae4aa1f95129e5e54670f1", hex);
}

// single character
void test_sha1_single_char(void) {
        char hex[41];
        hash("e", hex);
        ASSERT_STR_EQ("86f7e437faa5a7fce15d382a6e7e4f1bef1a0a63", hex);
}

// 64 bytes exactly (one full block)
void test_sha1_64_bytes(void) {
        char hex[41];
        hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqo", hex);
        ASSERT_STR_EQ("0d973364e78b0a0ae0a4a4d08b16c99e0e585667", hex);
}

// multiple update calls
void test_sha1_multi_update(void) {
        sha1_context ctx;
        sha1_init(&ctx);
        sha1_update(&ctx, (const u8*)"hello ", 6);
        sha1_update(&ctx, (const u8*)"world", 5);
        u8 out[SHA1_DIGEST_SIZE];
        sha1_final(&ctx, out);

        char hex[41];
        for (int i = 0; i < 20; i++)
                sprintf(hex + i * 2, "%02x", out[i]);
        hex[40] = '\0';

        ASSERT_STR_EQ("2aae6c35c94fcfb415dbe95f408b9ce91ee846ed", hex);
}

// all zeros
void test_sha1_zeros(void) {
        char hex[41];
        hash("\x00\x00\x00\x00", hex);
        ASSERT_STR_EQ("c8d7d0ef0eedfa82d2ea1aa592845b9a6d4b02b7", hex);
}

void test_sha1_long_msg(void) {
        // 1000 'a' characters
        char msg[1001];
        memset(msg, 'a', 1000);
        msg[1000] = '\0';
        char hex[41];
        hash(msg, hex);
        ASSERT_STR_EQ("291e9a6c66994949b57ba5e650361e98fc36b1ba", hex);
}

void register_sha_tests(void) {
        REGISTER_TEST(test_sha1_empty);
        REGISTER_TEST(test_sha1_abc);
        REGISTER_TEST(test_sha1_hello_world);
        REGISTER_TEST(test_sha1_56_bytes);
        REGISTER_TEST(test_sha1_single_char);
        REGISTER_TEST(test_sha1_64_bytes);
        REGISTER_TEST(test_sha1_multi_update);
        REGISTER_TEST(test_sha1_zeros);
        REGISTER_TEST(test_sha1_long_msg);
}
