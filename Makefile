# This doesn't make the test files, just the library .a file.

all:
	mkdir -p target
	gcc -c src/endian.c src/nbtcore.c src/anvillib.c
	mv *.o target
	ar cr target/libanvil.a target/anvillib.o target/nbtcore.o target/endian.o

test:
	gcc -lz test/test.c src/endian.c src/nbtcore.c src/anvillib.c


leveldat:
	gcc -lz test/test_leveldat.c src/endian.c src/nbtlib.c src/nbtcore.c src/anvillib.c
