
mycop: y.tab.o lex.yy.o
	gcc y.tab.o lex.yy.o -o mycop
y.tab.o: myparse.y
	yacc -d -v myparse.y
	gcc -c y.tab.c
lex.yy.o: mylex.l
	flex mylex.l
	gcc -c lex.yy.c


