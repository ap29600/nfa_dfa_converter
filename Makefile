CFLAGS=-Wall -Wextra -std=c11 -O3 -march=native

default: bin/dfa

clean: 
	$(RM) bin/dfa
	$(RM) *.dot
	$(RM) regex.c


bin/dfa: src/*.c src/*.h bin
	gcc $(CFLAGS) src/*.c -o bin/dfa

bin:
	mkdir bin
