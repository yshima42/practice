#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
	SERVER_PORT = 12345,
	NQUEUESIZE = 5,
	MAXNCLIENTS = 10,
};

int clients[MAXNCLIENTS];
int nclients = 0;

void sorry(int ws) {
	char *message = "Sorry, it's full.\n";

	write(ws, message, strlen(message));
}

void delete_client(int ws) {
	int i;

	for (i = 0; i < nclients; i++) {
		if (clients[i] == ws) {
			clients[i] = clients[nclients - 1];
			nclients--;
			break;
		}
	}
}

void talks (int ws) {
	int i, cc;
	char c;
	
	do {
		if ((cc = read(ws, &c, 1)) == -1) {
			perror("read");
			exit(1);
		} else if (cc == 0) {
			shutdown(ws, SHUT_RDWR);
			close(ws);
			delete_client(ws);
			fprintf(stderr, "Connection closed on descriptor %d. \n", ws);
			return ;
		}
		for (i = 0; i < nclients; i++)
			write(clients[i], &c, 1);
	} while (c != '\n');
}

int main(void)
{
	int s, ws, soval, cc;
	struct sockaddr_in sa, ca;
	socklen_t ca_len;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	soval = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &soval, sizeof(soval)) == -1) {
		perror("setsockopt");
		exit(1);
	}

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

	fprintf(stderr, "Ready.\n");

	for (;;) {
		pid_t forkcc;
		int status;

		while (waitpid(-1, &status, WNOHANG) > 0);

		fprintf(stderr, "Waiting for a connection...\n");
		ca_len = sizeof(ca);
		if ((ws = accept(s, (struct sockaddr *)&ca, &ca_len)) == -1) {
			perror("accept");
			exit(1);
		}
		fprintf(stderr, "Message sent.\n");

		if ((forkcc = fork()) == -1) {
			perror("fork");
			exit(1);
		} else if (forkcc == 0) {
			if (close(s) == -1) {
				perror ("child: close listening socket");
				exit(1);
			}
			fprintf(stderr, "sending the message...\n");

			if((cc = write(ws, message, strlen(message))) == -1) {
				perror("write");
				exit(1);
			}
			fprintf(stderr, "Message sent.\n");

		if (shutdown(ws, SHUT_RDWR) == -1) {
			perror("shutdown");
			exit(1);
		}

		if (close(ws) == -1) {
			perror("close");
			exit(1);
		}
		exit(0);
		}
		if (close(ws) == -1) {
			perror("close");
			exit(1);
		}

	}
}

