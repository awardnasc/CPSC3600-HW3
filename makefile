all: starLord chopper ahsoka

CC=gcc
CFLAGS=-I. -std=gnu99 -g
starLordDEPS = starLord.c AddressUtility.c
chopperDEPS = chopper.c AddressUtility.c
ahsokaDEPS = ahsoka.c AddressUtility.c

starLord: $(starLordDEPS)
	$(CC) $(CFLAGS) $(starLordDEPS) -o starLord

chopper: $(chopperDEPS) 
	$(CC) $(CFLAGS) $(chopperDEPS) -o chopper

ahsoka: $(ahsokaDEPS) 
	$(CC) $(CFLAGS) $(ahsokaDEPS) -o ahsoka

clean:
	rm -f starLord chopper ahsoka
