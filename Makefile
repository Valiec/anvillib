# This doesn't make the test files, just the library .a file.

.PHONY: clean lib test leveldat

lib: libanvil.a

libanvil.a:
	mkdir -p target
	gcc -c -Isrc src/endian.c src/nbtcore.c src/anvillib.c
	mv *.o target
	ar cr target/libanvil.a target/anvillib.o target/nbtcore.o target/endian.o

test: libanvil.a
	gcc -lz -Isrc test/test.c src/endian.c src/nbtcore.c src/anvillib.c -o target/test


leveldat: libanvil.a
	gcc -lz -Isrc test/test_leveldat.c src/endian.c src/nbtlib.c src/nbtcore.c src/anvillib.c -o target/leveldat

clean:
	rm -rf target/*