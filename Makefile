pygrep: pygrep.c pinyin.map.c
	$(CC) pygrep.c pinyin.map.c -o pygrep -std=c99 -Wall -lpcre2-8
