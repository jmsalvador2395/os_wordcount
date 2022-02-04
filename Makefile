pwordcount: pwordcount.o
	gcc -o pwordcount pwordcount.o

pwordcount.o: pwordcount.c
	gcc -c pwordcount.c

clean:
	rm .o
	rm pwordcount
