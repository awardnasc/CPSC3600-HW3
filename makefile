all: starLord chopper ahsoka

CC=gcc
CFLAGS=-I. -std=gnu99

DEPS = starLord.c AddressUtility.c
starLord: starLord.c AddressUtility.c
	$(CC) $(CFLAGS) $(DEPS) -o starLord

DEPS = chopper.c AddressUtility.c
chopper: chopper.c AddressUtility.c
	$(CC) $(CFLAGS) $(DEPS) -o chopper

DEPS = ahsoka.c AddressUtility.c
ahsoka: ahsoka.c AddressUtility.c
	$(CC) $(CFLAGS) $(DEPS) -o ahsoka

clean:
	rm -f starLord chopper ahsoka

