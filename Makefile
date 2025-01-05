CC = gcc
CFLAGS = -Wall -I./src -I./include -DDEBUG
LDFLAGS = -lpigpio -lpthread -lrt

SRCS = src/main.c src/arp_test.c src/spi.c src/register.c
OBJS = $(SRCS:.c=.o)
TARGET = arp_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
