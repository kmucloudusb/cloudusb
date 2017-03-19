main.out : fat_string.o
	gcc -o main.out main.c fat_string.o -D_FILE_OFFSET_BITS=64 -lfuse

fat_string.o : fat_string.c
	gcc -c -o fat_string.o fat_string.c

clean :
	rm *.o main.out