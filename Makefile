CFLAGS = -std=c99 -Wall -Wpedantic $(shell pkg-config --cflags libpcre2-8)
LDFLAGS = $(shell pkg-config --libs libpcre2-8)

pygrep : pygrep.c pinyin.map.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY : clean
clean :
	-rm pygrep
