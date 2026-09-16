CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread

SRCS = main.c stack.c size.c worker.c progress.c args.c delta.c
OBJS = $(SRCS:.c=.o)
TARGET = dut

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

veryclean:
	rm -f $(OBJS) $(TARGET)
