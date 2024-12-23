CC = gcc
CFLAGS = -Wall -I./inc
LDFLAGS = -lpigpio -lpthread -lrt

SRCS = main.c arp_test.c
OBJS = $(SRCS:.c=.o)
TARGET = arp_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 