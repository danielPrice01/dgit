.PHONY: all debug release clean

CC := gcc
RM := rm -f

TARGET := dgit

SRC_DIRS := src utils utils/SHA1

SRCS := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
OBJS := $(SRCS:.c=.o)

CFLAGS_COMMON := -O2 -Wall -Wextra -pedantic -std=c11 -D_DEFAULT_SOURCE
CFLAGS_COMMON += $(addprefix -I,$(SRC_DIRS))
CFLAGS_COMMON += -MMD -MP 

CFLAGS_DEBUG := -g -Og
CFLAGS_RELEASE := -O2 -DNDEBUG

all: debug

debug: CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
debug: $(TARGET)

release: CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_RELEASE)
release: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	$(RM) $(TARGET) $(OBJS)
	rm -rf .dgit
