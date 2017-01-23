CC = gcc
CFLAGS = -W -Wall
TARGET = main.out
OBJECTS = fat_access.o fat_cache.o fat_filelib.o fat_format.o fat_misc.o fat_string.o fat_table.o fat_write.o main.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm *.o main.out
