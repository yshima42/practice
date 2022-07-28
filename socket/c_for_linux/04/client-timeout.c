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
#include <fcntl.h>

int SetBlock(int fd, int flag)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		perror("fcntl");
		return (-1);
	}
	if (flag == 0) {
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}
	else if (flag == 1) {
		fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	}
	return 0;
}

int ClientSocket2(char *hostnm, char *portnm, int timeout_sec)
{
	struct in_addr addr;
	struct hostent *host;
	struct sockaddr_in server;
	struct servent *se;
	int soc, portno;

	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;

	if ((addr.s_addr = inet_addr(hostnm)) == -1) {
		host = gethostbyname(hostnm);
		if (host == NULL) {
			fprintf(stderr, "gethostbyname():error\n");
			return (-1);
		}
		memcpy(&addr, (struct in_addr *)*host->h_addr_list, sizeof(struct in_addr));
		fprintf(stderr, "addr = %s\n", inet_ntoa(addr));

		server.sin_addr = addr;
		if (isdigit(portnm[0])) {
			if ((portno = atoi(portnm)) <= 0) {
				fprintf(stderr, "bad port no\n");
				return -1;
			}
			server.sin_port = htons(portno);
		} else {
			server.sin_port = se->s_port;
		}
	}
	fprintf(stderr, "port = %d\n", ntohs(server.sin_port));
	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (timeout_sec < 0) {
		if (connect(soc, (struct sockaddr *)&server, sizeof(server)) == -1) {
			perror("connect");
			close(soc);
			return -1;
		}
		return (soc);
	} else {
		int width, val;
		socklen_t len;
		fd_set mask, write_mask, read_mask;
		struct timeval timeout;

		SetBlock(soc, 0);
		switch(connect(soc, (struct sockaddr *)&server, sizeof(server))) {
			case -1:
				if (errno != EINPROGRESS) {
					perror("connect");
					close(soc);
					return -1;
				}
				break;
			case 0:
				SetBlock(soc,1);
				return (soc);
			default:
				break;
		}

		width = 0;
		FD_ZERO(&mask);
		FD_SET(soc, &mask);
		width = soc + 1;
		timeout.tv_sec = timeout_sec;
		timeout.tv_usec = 0;
		while (1) {
			write_mask = mask;
			read_mask = mask;
			switch (select(width, &read_mask, &write_mask, NULL, &timeout)) {
				case -1:
					if (errno != EINTR) {
						perror("select");
						close(soc);
						return (-1);
					}
					break;
				case 0:
					fprintf(stderr, "select:timeout\n");
					close(soc);
					return -1;
				default:
					if (FD_ISSET(soc, &write_mask) || FD_ISSET(soc, &read_mask)) {
						len = sizeof(len);
						if (getsockopt(soc, SOL_SOCKET, SO_ERROR, &val, &len) != -1) {
							if (val == 0) {
								SetBlock(soc, 1);
								return (soc);
							} else {
								fprintf(stderr, "getsockopt:%d:%s\n", val, strerror(val));
								close (soc);
								return -1;
							}
						} else {
							perror("getsockopt");
							close(soc);
							return -1;
						}
					}
			}
		}
	}
}

int SendRecvLoop(int soc)
{
	int len;
	char buf[512];
	fd_set Mask, readOk;
	int width;
	struct timeval timeout;
	int error;

	FD_ZERO(&Mask);
	FD_SET(soc, &Mask);
	FD_SET(0, &Mask);
	width = soc + 1;

	error = 0;
	while (1) {
		readOk = Mask;
		timeout.tv_sec = 0;
		switch (select(width, (fd_set *)&readOk, NULL, NULL, &timeout)) {
			case -1:
				perror("select");
				break;
			case 0:
				break;
			default:
				if (FD_ISSET(0, &readOk)) {
					fgets(buf, sizeof(buf), stdin);
					if (feof(stdin)){
						error = 1;
						break;
					}
					if ((len = send(soc, buf, strlen(buf), 0)) <0) {
						perror("send");
						error = 1;
						break;
					}
				}
				if (FD_ISSET(soc, &readOk)) {
					if ((len = recv(soc, buf, sizeof(buf), 0)) < 0) {
						perror("recv");
						error = 1;
						break;
					}
					if (len == 0) {
						fprintf(stderr, "recv:EOF\n");
						error = 1;
						break;
					}
					buf[len] = '\0';
					printf("> %s", buf);
				}
		}
		if (error) {
			break;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int soc;
	if (argc <= 3) {
		fprintf(stderr, "client server-host port timeout-sec(-1:no-timeout)\n");
	}
	soc = ClientSocket2(argv[1], argv[2], atoi(argv[3]));
	if (soc == -1) {
		fprintf(stderr, "clientsocket2():error\n");
		return -1;
	}
	SendRecvLoop(soc);
	close(soc);
	return 0;
}
