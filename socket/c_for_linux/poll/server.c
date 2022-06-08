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
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int ServerSocket(char *portnm) {
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
  if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)) !=
      0) {
    perror("setsockopt");
    close(soc);
    return (-1);
  }

  if (bind(soc, (struct sockaddr *)&my, sizeof(my)) == -1) {
    perror("bind");
    close(soc);
    return (-1);
  }

  if (listen(soc, SOMAXCONN) == -1) {
    perror("listen");
    close(soc);
    return (-1);
  }

  return (soc);
}

#define MAX_CHILD (20)

int RecvSendOne(int acc, int child_no, char *buf) {
  char *ptr;
  int len;

  if ((len = recv(acc, buf, sizeof(buf), 0)) < 0) {
    perror("recv");
    return (-1);
  }
  if (len == 0) {
    fprintf(stderr, "[child%d]recv:EOF\n", child_no);
    return (0);
  }
  buf[len] = '\0';
  if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
    *ptr = '\0';
  }
  fprintf(stderr, "[child%d]%s\n", child_no, buf);
  strcat(buf, ":OK\r\n");
  len = strlen(buf);
  if ((len = send(acc, buf, len, 0)) < 0) {
    perror("send");
    return (-1);
  }

  return (1);
}

char request_message[1024];

int AcceptLoop(int soc) {
  int acc;
  socklen_t len;
  struct sockaddr_in from;
  int child[MAX_CHILD];
  int child_no;
  struct pollfd targets[MAX_CHILD + 1];
  int i, j, count, nready, pos, ret;

  for (i = 0; i < MAX_CHILD; i++) {
    child[i] = -1;
  }
  child_no = 0;

  while (1) {
    count = 0;
    targets[count].fd = soc;
    targets[count].events = POLLIN;
    count++;
    for (i = 0; i < child_no; i++) {
      if (child[i] != -1) {
        targets[count].fd = child[i];
        targets[count].events = POLLIN;
        count++;
      }
    }
    fprintf(stderr, "<<child count:%d>>\n", count - 1);

    nready = poll(targets, count, 10 * 1000);
    if (nready == -1) {
      perror("poll");
    } else if (nready > 0) {
      if (targets[0].revents & POLLIN) {
        len = sizeof(from);
        acc = accept(soc, (struct sockaddr *)&from, &len);
        if (acc < 0) {
          if (errno != EINTR) {
            perror("accept");
          }
        } else {
          fprintf(stderr, "accept:%s:%d\n", inet_ntoa(from.sin_addr),
                  ntohs(from.sin_port));

          pos = -1;
          for (i = 0; i < child_no; i++) {
            if (child[i] == -1) {
              pos = i;
              break;
            }
          }
          if (pos == -1) {
            if (child_no + 1 >= MAX_CHILD) {
              fprintf(stderr, "child is full : cannot accept\n");
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
      for (i = 1; i < count; i++) {
        if (targets[i].revents & (POLLIN | POLLERR)) {
          ret = RecvSendOne(targets[i].fd, i - 1, request_message);
          if (ret <= 0) {
            close(targets[i].fd);
            for (j = 0; j < child_no; j++) {
              if (child[j] == targets[i].fd) {
                child[j] = -1;
                break;
              }
            }
          }
        }
      }
    }
  }
  return (0);
}

int parseRequestMessage(char *method, char *target, char *request_message) {

    char *line;
    char *tmp_method;
    char *tmp_target;
    
    /* リクエストメッセージの１行目のみ取得 */
    line = strtok(request_message, "\n");

    /* " "までの文字列を取得しmethodにコピー */
    tmp_method = strtok(line, " ");
    if (tmp_method == NULL) {
        printf("get method error\n");
        return -1;
    }
    strcpy(method, tmp_method);

    /* 次の" "までの文字列を取得しtargetにコピー */
    tmp_target = strtok(NULL, " ");
    if (tmp_target == NULL) {
        printf("get target error\n");
        return -1;
    }
    strcpy(target, tmp_target);

    return 0;
}

int main(int argc, char *argv[]) {
  int soc;

  if (argc <= 1) {
    fprintf(stderr, "server1 port\n");
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
