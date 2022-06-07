#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

enum {
	SERVER_PORT = 12345,
	NQUEUESIZE = 5,
	MAXNCLIENTS = 10,
};

int clients[MAXNCLIENTS];
int nclients = 0;

void sorry (int ws) {
	char *message = "sorry, its full.\n";

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

void talks(int ws) {
	int i, cc;
	char c;

	do {
		cc = read(ws, &c, 1);
		if (cc == 0) {
			shutdown(ws, SHUT_RDWR);
			close(ws);
			delete_client(ws);
			fprintf(stderr, "connection closed on descriptor %d.\n", ws);
			return;
		}
		for (i = 0; i < nclients; i++)
			write(clients[i], &c, 1);
	} while (c != '\n');
}

int main(void)
{
	int s, soval;
	struct sockaddr_in sa;

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
	listen(s, NQUEUESIZE);

	fprintf(stderr, "Ready.\n");

	for(;;) {
		int i, maxfd;
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		maxfd = s;

		for(i = 0; i < nclients; i++) {
			FD_SET(clients[i], &readfds);
			if (clients[i] > maxfd)
				maxfd = clients[i];
		}

		if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(s, &readfds)) {
			struct sockaddr_in ca;
			socklen_t ca_len;
			int ws;

			ca_len = sizeof(ca);
			if ((ws = accept(s, (struct sockaddr *)&ca, &ca_len)) == -1) {
				perror("accept");
				continue;
			}
			if (nclients >= MAXNCLIENTS) {
				sorry(ws);
				shutdown(ws, SHUT_RDWR);
				close(ws);
				fprintf(stderr, "Refused a new connection.\n");
			} else {
				clients[nclients] = ws;
				nclients++;
				fprintf(stderr, "Accepted a connection on descriptor %d.\n", ws);
			}
		}
		for (i = 0; i < nclients; i++) {
			if (FD_ISSET(clients[i], &readfds)) {
				talks(clients[i]);
				break;
			}
		}
	}
}
