CFLAGS=-Wall -Wextra -std=c11

default: clean pics

clean:
	$(RM) *.svg *.dot dfa

pics: initial.dot intermediate.dot minimised.dot
	dot -Tsvg initial.dot > initial.svg
	dot -Tsvg intermediate.dot > intermediate.svg
	dot -Tsvg minimised.dot > minimised.svg

%.dot: dfa
	./dfa

dfa: *.c
	gcc $(CFLAGS) -ggdb *.c -o dfa
