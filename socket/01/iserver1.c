#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#define SERVER_ADDR	"127.0.0.1"
#define SERVER_PORT 12345

enum {
	NQUEUESIZE = 5,
};

char *message = "hellohello ! \n";

void sendmessage(int s) {
	char buf[1024];

	while (fgets(buf, sizeof(buf), stdin) != NULL) 
		write(s, buf, strlen(buf));

	clearerr(stdin);
}

int main(void)
{
	int s, ws, soval, cc;
	struct sockaddr_in sa, ca;
	socklen_t ca_len;

	s = socket(AF_INET, SOCK_STREAM, 0);

	soval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &soval, sizeof(soval));

	memset(&sa, 0, sizeof(sa));
	sa.sin_len = sizeof(sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(SERVER_PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		perror("bind");
		exit(1);
	}
	
	if (listen(s, NQUEUESIZE)) {
		perror("listen");
		exit(1);
	}

	fprintf(stderr, "ready.\n");

	for (;;) {
		fprintf(stderr, "waiting for a connection...\n");
		ca_len = sizeof(ca);
		ws = accept(s, (struct sockaddr *)&ca, &ca_len);
		fprintf(stderr, "connectrion established.\n");

		fprintf(stderr, "sending the message...\n");

		sendmessage(ws);

		fprintf(stderr, "massage sent.\n");

		shutdown(ws, SHUT_RDWR);
		close(ws);
	}
}

