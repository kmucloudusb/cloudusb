CC = gcc
CFLAGS = -W -Wall
TARGET = main.out
OBJECTS = fat_string.o main.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm *.o main.out
