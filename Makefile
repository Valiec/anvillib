all:
	gcc -c endian.c nbtlib.c anvillib.c
	gcc -c endian.c nbtlib.c anvillib.c
	ar cr libanvil.a anvillib.o nbtlib.o endian.o