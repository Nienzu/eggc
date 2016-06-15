
mycop: calc3b.c test.tab.o lex.yy.o
	gcc -g test.tab.o lex.yy.o calc3b.c -o mycop
test.tab.o: test.y
	bison -d -v test.y
	gcc -g -c test.tab.c
lex.yy.o: test.lex
	flex test.lex
	gcc -g -c lex.yy.c


