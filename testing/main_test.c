#include <assert.h>
#define DTEST_IMPL
#include "SHA_tests.h"
#include "dtest.h"

void register_sha_tests(void);

int main(int argc, char** argv) {
        printf("\nSHA tests:");
        register_sha_tests();

        RUN_TESTS(argc, argv);
}
