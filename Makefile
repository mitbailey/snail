CC=gcc
CFLAGS= -std=gnu11 -O2 -I gpiodev/ -I include/
LDFLAGS= -lm -lpthread

COBJS=src/main.o \
		gpiodev/gpiodev.o

all: $(COBJS)
	$(CC) -o record_data.out $(COBJS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -vf $(COBJS)
	rm -vf *.out

spotless: clean
	rm -vf data/*
	\

CC = gcc
COBJS = src/main.o gpiodev/gpiodev.o
EDCFLAGS = -I ./ -I ./include/ -I ./gpiodev/ -Wall -pthread $(CFLAGS)
EDLDFLAGS := -lpthread -lm $(LDFLAGS)
TARGET = snail.out

all: $(COBJS)
	$(CC) $(EDCFLAGS) $(COBJS) -o $(TARGET) $(EDLDFLAGS)
	./$(TARGET)

%.o: %.c
	$(CC) $(EDCFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	$(RM) *.out
	$(RM) *.o
	$(RM) src/*.o

.PHONY: spotless

spotless:
	$(RM) gpiodev/*.o