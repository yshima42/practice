#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int ac, char *av[])
{
    int fd;
    if (ac < 3) {
        fprintf(stderr, "%s filename command \n", av[0]);
        exit(1);
    }
    fd = open (av[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    dup2(fd, STDOUT_FILENO);
    execvp(av[2], &av[2]);
}
