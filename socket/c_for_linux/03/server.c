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

#define MAX_CHILD 20

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

int RecvSendOne(int acc, int child_no)
{
	char buf[512], *ptr;
	int len;

	if ((len = recv(acc, buf, sizeof(buf), 0)) < 0) {
		perror("recv");
		return(-1);
	}
	if (len == 0) {
		fprintf(stderr, "[child%d]recv:EOF\n", child_no);
		return 0;
	}
	buf[len] = '\0';
	if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
		*ptr = '\0';
	}
	fprintf(stderr, "[child%d]%s\n", child_no, buf);
	strcat(buf, "OK\r\n");
	len = strlen(buf);
	if ((len = send(acc, buf, len, 0)) < 0) {
		perror("send");
		return -1;
	}
	return 1;
}

int AcceptLoop(int soc)
{
	int acc;
	socklen_t len;
	struct sockaddr_in from;
	int child[MAX_CHILD];
	int child_no;
	fd_set mask;
	int width;
	struct timeval timeout;
	int i, count, pos, ret;

	for (i = 0; i < MAX_CHILD; i++) {
		child[i] = -1;
	}
	child_no = 0;

	while (1) {
		FD_ZERO(&mask);
		FD_SET(soc, &mask);
		width = soc + 1;
		count = 0;

		for (i = 0; i < child_no; i++) {
			if (child[i] != -1) {
				FD_SET(child[i], &mask);
				if (child[i] + 1 > width) {
					width = child[i] + 1;
					count++;
				}
			}
		}
		fprintf(stderr, "<<child count:%d>>\n", count);
		
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		switch(select(width, (fd_set *)&mask, NULL, NULL, &timeout)) {
			case -1:
				perror("select");
				break;
			case 0:
				break;
			default:
				if (FD_ISSET(soc, &mask)) {
					len = sizeof(from);
					acc = accept(soc, (struct sockaddr *)&from, &len);
					if (acc < 0) {
						if (errno != EINTR) {
							perror("accept");
						}
					} else {
						fprintf(stderr, "accept:%s:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
						pos = -1;
						for (i = 0; i < child_no; i++) {
							if (child[i] == -1) {
								pos = i;
								break;
							}
						}
						if (pos == -1) {
							if (child_no + 1 >= MAX_CHILD) {
								fprintf(stderr, "child is full : cannnot accept\n");
								close(acc);
							} else {
								child_no++;
								pos = child_no - 1;
							}
						}
						if (pos != -1) {
							child[pos] = acc;
						}
					}
				}

				for (i = 0; i < child_no; i++) {
					if (child[i] != -1) {
						if (FD_ISSET(child[i], &mask)) {
							ret = RecvSendOne(child[i], i);
							if (ret <= 0) {
								close(child[i]);
								child[i] = -1;
							}
						}
					}
				}
		}
	}
	return 0;
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
				
