all: lib
SOURCE = btree.c
lib: btree.c
	gcc $(SOURCE)  -shared -o btree.so
