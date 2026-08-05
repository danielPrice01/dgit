CC := gcc

TARGET := dgit
TEST_TARGET := test_runner

SRC := $(wildcard src/*.c utils/*.c utils/SHA1/*.c)
TEST_SRC := $(wildcard testing/*.c)

OBJ := $(SRC:.c=.o)
TEST_OBJ := $(filter-out src/cli.o,$(OBJ)) $(TEST_SRC:.c=.o)
DEP := $(OBJ:.o=.d) $(TEST_SRC:.c=.d)

CPPFLAGS := -Isrc -Iutils -Iutils/SHA1 -D_DEFAULT_SOURCE
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -MMD -MP

.PHONY: all debug release test clean

all: debug

debug: CFLAGS += -g -Og
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

test: CFLAGS += -g -Og
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(TEST_TARGET) $(OBJ) $(TEST_OBJ) $(DEP)
	rm -rf .dgit

-include $(DEP)
