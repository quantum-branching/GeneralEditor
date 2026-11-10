CC = gcc
CFLAGS = -O3 -s -flto -std=c99

my_program: Main.c
	$(CC) $^ $(CFLAGS) -o QuantumsEditor
