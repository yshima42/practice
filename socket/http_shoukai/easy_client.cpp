#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

int RecvLine(int iSocket, char *szBuf, int iLen)
{
	int iBytesRead, iIdx, bNotDone;

	iBytesRead = recv(iSocket, &szBuf[0], 1, 0);
	iIdx = 1;
	bNotDone = true;

	while (bNotDone == true)
	{
		iBytesRead = recv(iSocket, &szBuf[iIdx], 1, 0);
		if (iBytesRead < 0) {
			return -1;
		}

		iIdx++;
		if ((szBuf[iIdx - 2] == '\r') && (szBuf[iIdx -1] == '\n')) {
			bNotDone = false;
		}
		if (iIdx == iLen) {
			return -1;
		}
	}
	szBuf[iIdx - 2] = '\0';
	return true;

}

int main(int ac, char *av[])
{
	int s, rc;
	char szBuf[256];
	struct sockaddr_in server;

	if (ac != 2) {
		std::cerr << "error! incorrect number of arguments." << std::endl;
		return 0;
	}
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		std::cerr << "error! cannot create socket" << std::endl;
		return 0;
	}

	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(7777);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	rc = connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	if (rc < 0) {
		std::cerr << "error! connect failed." << std::endl;
		return 0;
	}

	strcpy(szBuf, "HELLO\r\n");
	rc = send(s, szBuf, strlen(szBuf), 0);
	if (rc < 0) {
		std::cerr << "error! send failed" << std::endl;
		return 0;
	}

	/* rc = RecvLine(s, szBuf, 256); */
	/* if (rc < 0) { */
	/* 	std::cerr << "error! RecvLine failed." << std::endl; */
	/* 	return 0; */
	/* } */

	/* if (strcmp(szBuf, "OK") != 0) { */
	/* 	std::cerr << "error! unknown reply from server." << std::endl; */
	/* 	return 0; */
	/* } */

	/* strcpy(szBuf, "GOODBYE\r\n"); */
	/* rc = send(s, szBuf, strlen(szBuf), 0); */
	/* if (rc < 0) { */
	/* 	std::cerr << "error! send failed." << std::endl; */
	/* 	return 0; */
	/* } */

	/* rc = RecvLine(s, szBuf, 256); */
	/* if (rc < 0) { */
	/* 	std::cerr << "error! recvLine failed." << std::endl; */
	/* 	return 0; */
	/* } */

	/* if (strcmp(szBuf, "OK") != 0) */
	/* { */
	/* 	std::cerr << "error! unknown reply from server." << std::endl; */
	/* 	return 0; */
	/* } */

	close(s);
}
