project: editEX.o buff.o 
	cc editEX.o buff.o  -o project -lncurses
editEX.o: editEX.c buff.h
	cc -Wall -c editEX.c -lncurses
buff.o: buff.c buff.h key_values.h
	cc -Wall -c buff.c -lncurses

