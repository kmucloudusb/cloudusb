CC = gcc
CFLAGS =
TARGET = ../../bin/main.out
OBJECTS = fat_access.o fat_cache.o fat_filelib.o fat_format.o fat_misc.o fat_string.o fat_write.o fat_table.o fat_custom.o main.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	rm *.o

clean :
	rm $(TARGET)
