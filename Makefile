CC = gcc
COBJS = src/main.o gpiodev/gpiodev.o
EDCFLAGS = -I ./ -I ./include/ -I ./gpiodev/ -Wall -pthread $(CFLAGS)
EDLDFLAGS := -lpthread -lm $(LDFLAGS)
TARGET = snail.out

all: $(COBJS)
	$(CC) $(EDCFLAGS) $(COBJS) -o $(TARGET) $(EDLDFLAGS)
	# ./$(TARGET)

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