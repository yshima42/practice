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

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define SIZE (5*1024)

/* ファイルサイズを取得する */
unsigned int getFileSize(const char *path) {
    int size, read_size;
    char read_buf[SIZE];
    FILE *f;

    f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }

    size = 0;
    do {
        read_size = fread(read_buf, 1, SIZE, f);
        size += read_size;
    } while(read_size != 0);

    fclose(f);

    return size;
}

/*
 * リクエストメッセージを受信する
 * sock：接続済のソケット
 * request_message：リクエストメッセージを格納するバッファへのアドレス
 * buf_size：そのバッファのサイズ
 * 戻り値：受信したデータサイズ（バイト長）
 */
int recvRequestMessage(int sock, char *request_message, unsigned int buf_size) {
    int recv_size;
    
    recv_size = recv(sock, request_message, buf_size, 0);
	printf("%s", request_message);

    return recv_size;
}

/*
 * リクエストメッセージを解析する（今回はリクエスト行のみ）
 * method：メソッドを格納するバッファへのアドレス
 * target：リクエストターゲットを格納するバッファへのアドレス
 * request_message：解析するリクエストメッセージが格納されたバッファへのアドレス
 * 戻り値：成功時は0、失敗時は-1
 */
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

/*
 * リクエストに対する処理を行う（今回はGETのみ）
 * body：ボディを格納するバッファへのアドレス
 * file_path：リクエストターゲットに対応するファイルへのパス
 * 戻り値：ステータスコード（ファイルがない場合は404）
 */
int getProcessing(char *body, char *file_path) {

    FILE *f;
    int file_size;

    /* ファイルサイズを取得 */
    file_size = getFileSize(file_path);
    if (file_size == 0) {
        /* ファイルサイズが0やファイルが存在しない場合は404を返す */
        printf("getFileSize error\n");
        return 404;
    }

    /* ファイルを読み込んでボディとする */
    f = fopen(file_path, "r");
    fread(body, 1, file_size, f);
    fclose(f);

    return 200;
}

/*
 * レスポンスメッセージを作成する
 * response_message：レスポンスメッセージを格納するバッファへのアドレス
 * status：ステータスコード
 * header：ヘッダーフィールドを格納したバッファへのアドレス
 * body：ボディを格納したバッファへのアドレス
 * body_size：ボディのサイズ
 * 戻り値：レスポンスメッセージのデータサイズ（バイト長）
 */
int createResponseMessage(char *response_message, int status, char *header, char *body, unsigned int body_size) {

    unsigned int no_body_len;
    unsigned int body_len;

    response_message[0] = '\0';

    if (status == 200) {
        /* レスポンス行とヘッダーフィールドの文字列を作成 */
        sprintf(response_message, "HTTP/1.1 200 OK\r\n%s\r\n", header);

        no_body_len = strlen(response_message);
        body_len = body_size;

        /* ヘッダーフィールドの後ろにボディをコピー */
        memcpy(&response_message[no_body_len], body, body_len);
    } else if (status == 404) {
        /* レスポンス行とヘッダーフィールドの文字列を作成 */
        sprintf(response_message, "HTTP/1.1 404 Not Found\r\n%s\r\n", header);

        no_body_len = strlen(response_message);
        body_len = 0;
    } else {
        /* statusが200・404以外はこのプログラムで非サポート */
        printf("Not support status(%d)\n", status);
        return -1;
    }

    return no_body_len + body_len;
}

/*
 * レスポンスメッセージを送信する
 * sock：接続済のソケット
 * response_message：送信するレスポンスメッセージへのアドレス
 * message_size：送信するメッセージのサイズ
 * 戻り値：送信したデータサイズ（バイト長）
 */
int sendResponseMessage(int sock, char *response_message, unsigned int message_size) {

    int send_size;
    
    send_size = send(sock, response_message, message_size, 0);

    return send_size;
}

void showMessage(char *message, unsigned int size) {
/* コメントを外せばメッセージが表示される
    unsigned int i;

    printf("Show Message\n\n");

    for (i = 0; i < size; i++) {
        putchar(message[i]);
    }
    printf("\n\n");
*/
}

/*
 * HTTPサーバーの処理を行う関数
 * sock：接続済のソケット
 * 戻り値：0
 */
int httpServer(int sock) {

    int request_size, response_size;
    char request_message[SIZE];
    char response_message[SIZE];
    char method[SIZE];
    char target[SIZE];
    char header_field[SIZE];
    char body[SIZE];
    int status;
    unsigned int file_size;
    

    while (1) {

        /* リクエストメッセージを受信 */
        request_size = recvRequestMessage(sock, request_message, SIZE);
        if (request_size == -1) {
            printf("recvRequestMessage error\n");
            break;
        }

        if (request_size == 0) {
            /* 受信サイズが0の場合は相手が接続閉じていると判断 */
            printf("connection ended\n");
            break;
        }

        /* 受信した文字列を表示 */
        showMessage(request_message, request_size);

        /* 受信した文字列を解析してメソッドやリクエストターゲットを取得 */
        if (parseRequestMessage(method, target, request_message) == -1) {
            printf("parseRequestMessage error\n");
            break;
        }

        /* メソッドがGET以外はステータスコードを404にする */
        if (strcmp(method, "GET") == 0) {
            if (strcmp(target, "/") == 0) {
                /* /が指定された時は/index.htmlに置き換える */
                strcpy(target, "/index.html");
            }

            /* GETの応答をするために必要な処理を行う */
            status = getProcessing(body, &target[1]);
        } else {
            status = 404;
        }

        /* ヘッダーフィールド作成(今回はContent-Lengthのみ) */
        file_size = getFileSize(&target[1]);
        sprintf(header_field, "Content-Length: %u\r\n", file_size);

        /* レスポンスメッセージを作成 */
        response_size = createResponseMessage(response_message, status, header_field, body, file_size);
        if (response_size == -1) {
            printf("createResponseMessage error\n");
            break;
        }

        /* 送信するメッセージを表示 */
        showMessage(response_message, response_size);

        /* レスポンスメッセージを送信する */
        sendResponseMessage(sock, response_message, response_size);
        
    }

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
          //ret = RecvSendOne(targets[i].fd, i - 1, request_message);
		  ret = httpServer(targets[i].fd);
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
  close(soc);

  return (0);
}
