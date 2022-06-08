#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int ClientSocket(char *hostnm, char *portnm) {
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
  }
  fprintf(stderr, "addr=%s\n", inet_ntoa(addr));
  server.sin_addr = addr;
  if (isdigit(portnm[0])) {
    if ((portno = atoi(portnm)) <= 0) {
      fprintf(stderr, "bad port no\n");
      return (-1);
    }
    server.sin_port = htons(portno);
  } else {
    if ((se = getservbyname(portnm, "tcp")) == NULL) {
      fprintf(stderr, "getservbyname():error\n");
      return (-1);
    } else {
      server.sin_port = se->s_port;
    }
  }
  fprintf(stderr, "port=%d\n", ntohs(server.sin_port));

  if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return (-1);
  }
  if (connect(soc, (struct sockaddr *)&server, sizeof(server)) == -1) {
    perror("connect");
    close(soc);
    return (-1);
  }
  return (soc);
}

int SendRecvLoop(int soc) {
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
          if (feof(stdin)) {
            error = 1;
            break;
          }
          if ((len = send(soc, buf, strlen(buf), 0)) < 0) {
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

int main(int argc, char *argv[]) {
  int soc;
  if (argc <= 2) {
    fprintf(stderr, "client server-host port\n");
    return 1;
  }
  soc = ClientSocket(argv[1], argv[2]);
  if (soc == -1) {
    fprintf(stderr, "clientsocket():error\n");
    return -1;
  }
  SendRecvLoop(soc);
  close(soc);
  return 0;
}
