FILES = binary.tab.o lex.yy.c main.cc AST/AST.cpp AST/Environment.cpp AST/Data.cpp AST/Ret.cpp AST/Comp.cpp AST/Symbols.cpp

parser: $(FILES)
		g++ -std=c++11 -g -o int $(FILES)
binary.tab.o: binary.tab.cc
		g++ -std=c++11 -g -c binary.tab.cc
binary.tab.cc: binary.yy
		bison binary.yy --report=all
lex.yy.c: binary.ll binary.tab.cc
		flex binary.ll
clean:
		rm -f binary.tab.* lex.yy.c* int binary.output parse.dot.pdf parse.dot cfg.dot cfg.dot.pdf stack.hh target target.cc