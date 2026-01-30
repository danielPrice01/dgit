.PHONY: all clean

CC := clang
CFLAGS := -O2 -Wall -Wextra -pedantic -std=c11
RM := rm -f

TARGET := dgit
SRCS := cli.c SHA1.c handler.c
OBJS := $(SRCS:.c=.o)
HEADERS := SHA1.h utils.h handler.h typedefs.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJS)
	rm -rf .dgit
