#include <stdio.h>
#include <string.h>

int main(int ac, char *av[], char *envp[])
{
	int x;

	x = strcmp(0, " .html");
	printf("%d\n", x);
}
