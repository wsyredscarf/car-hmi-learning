CC = gcc
CFLAGS = -Wall -g -std=gnu99   # <-- 就是这里，加上 -std=gnu99
TARGET = app
SRCS = main.c ring_buffer.c uart.c modbus_parser.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f $(OBJS) $(TARGET)