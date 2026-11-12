CC = gcc
CFLAGS = -O3 -s -flto -std=c99

.phony: install uninstall clean

Quantix: Main.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm Quantix

install: Quantix
	install -m 755 Quantix /usr/bin/Quantix

uninstall:
	rm /usr/bin/Quantix