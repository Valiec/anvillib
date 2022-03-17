# This doesn't make the test files, just the library .a file.

all:
	gcc -c endian.c nbtcore.c anvillib.c
	gcc -c endian.c nbtcore.c anvillib.c
	ar cr libanvil.a anvillib.o nbtcore.o endian.o

test:
	gcc -lz test.c endian.c nbtcore.c anvillib.c


leveldat:
	gcc -lz test_leveldat.c endian.c nbtlib.c nbtcore.c anvillib.c
