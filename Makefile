default: clean pics

clean:
	$(RM) *.svg *.dot

pics: initial.dot intermediate.dot minimised.dot
	dot -Tsvg initial.dot > initial.svg
	dot -Tsvg intermediate.dot > intermediate.svg
	dot -Tsvg minimised.dot > minimised.svg

%.dot: dfa
	./dfa

dfa: *.c
	gcc -O3 -march=native *.c -o dfa
