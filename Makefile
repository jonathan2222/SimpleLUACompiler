FILES = binary.tab.o lex.yy.c main.cc AST/AST.cpp AST/Data.cpp AST/Comp.cpp AST/Symbols.cpp

parser: $(FILES)
		g++ -std=c++11 -g -o comp $(FILES)
binary.tab.o: binary.tab.cc
		g++ -std=c++11 -g -c binary.tab.cc
binary.tab.cc: binary.yy
		bison binary.yy --report=all
lex.yy.c: binary.ll binary.tab.cc
		flex binary.ll

targetE: target.cc
		g++ target.cc -o target -lm && ./target

targetDC: target.c
		gcc target.c -o target -lm && ./target

clean:
		rm -f binary.tab.* lex.yy.c* comp binary.output parse.dot.pdf parse.dot cfg.dot cfg.dot.pdf stack.hh target target.c target.cc