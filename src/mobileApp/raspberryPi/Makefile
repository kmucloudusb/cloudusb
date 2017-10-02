CC = gcc
CFLAGS = -W
TARGET = bluetoothEcho
LLIB = -lbluetooth

$(TARGET) : bluetoothEcho.o
	$(CC) $(CFLAGS) -o $(TARGET) bluetoothEcho.o $(LLIB)

bluetoothEcho.o : bluetoothEcho.c
	$(CC) $(CFLAGS) -c -o bluetoothEcho.o bluetoothEcho.c

