#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE 1024

int transfer(int sock) {
	char send_buf[BUF_SIZE], recv_buf;
	int send_size, recv_size;

	while(1) {
		printf("Input Message...\n");
		scanf("%s", send_buf);

		send_size = send(sock, send_buf, strlen(send_buf) + 1, 0);
		if (send_size == -1) {
			printf("send error\n");
			break;
		}

		recv_size = recv(sock, &recv_buf, 1, 0);
		if (recv_size == -1) {
			printf("recv error\n");
			break;
		}
		
		if (recv_size == 0) {
			printf("connection ended\n");
			break;
		}

		if (recv_buf == 0) {
			printf("Finish connection\n");
			break;
		}
	}
	return 0;
}

int main(void) {
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("sock error\n");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	printf("start connect...\n");
	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("connect error\n");
		close(sock);
		return -1;
	}
	printf("finish connect!\n");

	transfer(sock);

	close(sock);
	return 0;
}

