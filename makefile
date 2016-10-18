all: caseConverter bellower frank 

CC=gcc
CFLAGS=-I. -std=gnu99
DEPS = caseConverter.h

%.o: %.c $(DEPS)
	$(CC) -c -o  $@ $< $(CFLAGS)

caseConverter: caseConverter.c AddressUtility.c
	gcc -o caseConverter -std=gnu99 caseConverter.c AddressUtility.c -I.

bellower: bellower.c AddressUtility.c
	gcc -o bellower -std=gnu99 bellower.c AddressUtility.c -I.

frank: frank.c AddressUtility.c
	gcc -o frank -std=gnu99 frank.c AddressUtility.c -I.

clean:
	rm -f frank bellower caseConverter

