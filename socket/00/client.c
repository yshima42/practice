#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 12345

int main(void)
{
	int s, cc;
	struct sockaddr_in sa;
	char buf[1024];

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_len = sizeof(sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(SERVER_PORT);
	sa.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	
	fprintf(stderr, "Connecting to the server...\n");
	if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		perror("connect");
		exit(1);
	}
	fprintf(stderr, "Connected.\n");

	fprintf(stderr, "Message from the server:\n\n");
	while ((cc = read(s, buf, sizeof(buf))) > 0)
		write(1, buf, cc);
	if (cc == -1) {
		perror("read");
		exit(1);
	}
	fprintf(stderr, "\n\nFinished receiving.\n");

	if (shutdown(s, SHUT_RDWR) == -1) {
		perror("shutdown");
		exit(1);
	}

	if (close(s) == -1) {
		perror("close");
		exit(1);
	}
	return 0;

}
