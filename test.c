#include <stdio.h>
#include <stdlib.h>

int main (void) {

	char *test1;
	test1 = malloc(sizeof("hello"));

	char test2[5] = "hello";

	printf("size of test1 with just 'test1' is %li\n", sizeof(test1));
	printf("size of test2 is %li\n", sizeof(test2));

	return 0;
}
