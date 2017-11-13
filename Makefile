CFLAGS =-std=c99 -Wall -Wpedantic
LDFLAGS = -lpcre2-8

# XXX How to apply -DDEBUG via Make?
pygrep: pygrep.c pinyin.map.c
	$(CC) $(CFLAGS) $(LDFLAGS) pygrep.c pinyin.map.c -o pygrep
