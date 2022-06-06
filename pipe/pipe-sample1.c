#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd[2], nbytes;
    char buf[1024];

    if (pipe (fd) == -1)
        exit(1);
    write(fd[1], "hello\n", 6);
    nbytes = read(fd[0], buf, sizeof(buf));
    write(STDOUT_FILENO, buf, nbytes);
    close(fd[0]);
    close(fd[1]);
}
