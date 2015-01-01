
tdparser:	tdparser.y tdparser.l timezones.cc
	bison -d tdparser.y
	lex tdparser.l
	g++ -o tdparser tdparser.tab.c lex.yy.c timezones.cc

install: tdparser
	-mv /usr/local/bin/tdparser /usr/local/bin/tdparser-
	cp tdparser /usr/local/bin
clean:
	rm -f *.o tdparser *.tab.[ch] lex.yy.c
