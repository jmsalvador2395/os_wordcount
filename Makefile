pwordcount: pwordcount.o
	gcc -o pwordcount pwordcount.o

pwordcount.o: pwordcount.c
	gcc -c pwordcount.c

debug:
	gcc -g -o pwordcount pwordcount.c

clean:
	rm *.o
