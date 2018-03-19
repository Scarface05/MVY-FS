all:mvyfs.c
		gcc -o mvyfs general.c file_func.c dir_func.c mvyfs.c -lm -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`

run:mvyfs
	./mvyfs