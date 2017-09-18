TARGET = sort
LIBS = -lm
CC = g++
CFLAGS = -g -Wall -O0 -std=c++11 -march=native -masm=intel

.PHONY: default all clean

default: $(TARGET)
all: default

asm:
	$(CC) -Wa,-adhln  -std=c++11 -g -march=native -masm=intel sort.c > sort_asm.s

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# .PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
