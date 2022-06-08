#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

int ServerSocket(char *portnm)
{
	int soc, portno, opt;
	struct servent *se;
	struct sockaddr_in my;

	memset((char *)&my, 0, sizeof(my));
	my.sin_family = AF_INET;
	my.sin_addr.s_addr = htonl(INADDR_ANY);

	if (isdigit(portnm[0])) {
		if ((portno = atoi(portnm)) <= 0) {
			fprintf(stderr, "bad port no\n");
			return (-1);
		}
		my.sin_port = htons(portno);
	} else {
		if ((se = getservbyname(portnm, "tcp")) == NULL) {
			fprintf(stderr, "getservbyname():error\n");
			return (-1);
		} else {
			my.sin_port = se->s_port;
		}
	}

	fprintf(stderr, "port=%d\n", ntohs(my.sin_port));
	
	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	opt = 1;
	if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)) != 0) {
		perror("setsoctopt");
		close (soc);
		return (-1);
	}

	if (bind(soc, (struct sockaddr *)&my, sizeof(my)) == -1) {
		perror("bind");
		close(soc);
		return(-1);
	}

	if (listen(soc, SOMAXCONN) == -1) {
		perror("listen");
		close(soc);
		return (-1);
	}
	return (soc);
}

int SendRecvLoop(int acc)
{
	char buf[512], *ptr;
	int len;

	while (1) {
		if ((len = recv(acc, buf, sizeof(buf), 0)) < 0) {
				perror("recv");
				break;
		}
		if (len == 0) {
			fprintf(stderr, "recv:EOF\n");
			break;
		}

		buf[len] = '\n';
		if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
			*ptr = '\0';
		}

		fprintf(stderr, "[client]%s\n", buf);

		strcat(buf, ":OK\r\n");
		len = strlen(buf);

		if ((len = send(acc, buf, len, 0)) < 0) {
				perror("send");
				break;
		}
	}
	return 0;
}

int AcceptLoop(int soc)
{
	int acc;
	socklen_t len;
	struct sockaddr_in from;

	while (1) {
		len = sizeof(from);

		acc = accept(soc, (struct sockaddr *)&from, &len);
		if (acc < 0) {
			if (errno != EINTR) {
				perror("accept");
			}
		} else {
			fprintf(stderr, "accept:%s:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
			SendRecvLoop(acc);

			close(acc);
			acc = 0;
		}
	}
	return (0);
}

int main(int argc, char *argv[])
{
	int soc;
	if (argc <= 1) {
		fprintf(stderr, "server port\n");
		return (1);
	}

	soc = ServerSocket(argv[1]);
	if (soc == -1) {
		fprintf(stderr, "ServerSocket(%s):error\n", argv[1]);
		return (-1);
	}
	fprintf(stderr, "ready for accept\n");
	AcceptLoop(soc);
	close(soc);
	return (0);
}
				
