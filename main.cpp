#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"

int main(int argc, char* args[])
{
	char str1[20];
	printf("running.....\n");
	if (argc < 2)
	{
		printf("no rom file selected");
		return 0;
	}
	loadCart(args[1]);
	printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));
	return 0;
}