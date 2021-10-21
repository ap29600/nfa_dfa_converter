CFLAGS=-Wall -Wextra -std=c11

default: rm_pics pics

rm_pics:
	$(RM) *.svg *.dot

clean: rm_pics
	$(RM) dfa

pics: initial.dot intermediate.dot minimised.dot
	dot -Tsvg initial.dot > initial.svg
	dot -Tsvg intermediate.dot > intermediate.svg
	dot -Tsvg minimised.dot > minimised.svg

%.dot: dfa
	./dfa

dfa: *.c *.h
	gcc $(CFLAGS) -ggdb *.c -o dfa
