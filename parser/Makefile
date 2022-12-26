all: parser

parser:scanner lemon
	./lemon deckgrammar.y
	cp deckgrammar.c ../src/parser/deckgrammar.inc
	cp deckgrammar.h ../src/parser/deckgrammar.hpp

lemon: 
	gcc -o lemon lemon.c
	
scanner:
	ragel deckscanner.rl
	cp deckscanner.c ../src/parser/internal/deckscanner.cpp

clean:
	rm -f deckgrammar.c deckgrammar.h deckgrammar.out deckscanner.c lemon