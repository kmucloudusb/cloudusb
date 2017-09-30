CC = gcc
CFLAGS =
TARGET = ../../bin/main.out
OBJECTS = kernel.o file_system.o main.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	rm *.o

clean :
	rm $(TARGET)
