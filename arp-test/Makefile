CC = gcc
CFLAGS = -Wall -I./inc -DDEBUG_MODE # -DDEBUG_MODE is used to enable debug mode
LDFLAGS = -lpigpio -lpthread -lrt

SRCS = main.c arp_test.c spi.c register.c
OBJS = $(SRCS:.c=.o)
TARGET = arp_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
