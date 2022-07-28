#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

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

void TalkToClient(int iSocket)
{
	int iRc, bNotDone;
	char szBuf[256], szOk[] = "OK", szErr[] = "ERR";

	bNotDone = true;

	while (bNotDone == true) {
		iRc = RecvLine(iSocket, szBuf, 256);
		if (iRc < 0) {
			std::cerr << "Error! recvline failed." << std::endl;
			bNotDone = false;
		}
		
		if (strcmp(szBuf, "HELLO") == 0) {
			printf("cameeeeee!!\n");
			iRc = send(iSocket, szOk, strlen(szOk), 0);
			if (iRc < 0) {
				std::cerr << "error! send failed" << std::endl;
				bNotDone = false;
			}
		} else if (strcmp(szBuf, "GOODBYE") == 0) {
			iRc = send(iSocket, szOk, strlen(szOk), 0);
			if (iRc < 0) {
				std::cerr << "Error! send failed." << std::endl;
				bNotDone = false;
			}
			bNotDone = false;
		} else {
			send(iSocket, szErr, strlen(szErr), 0);
			bNotDone = false;
		}
	}
	close(iSocket);
}


int main(int argc, char *argv[])
{
	int s, rc, c;
	socklen_t len;
	struct sockaddr_in server, client;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		std::cerr << "Error! cannot create socket." << std::endl;
		return 0;
	}
	len = sizeof(struct sockaddr_in);
	memset(&server, 0, len);
	server.sin_family = AF_INET;
	server.sin_port = htons(7777);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	rc = bind(s, (struct sockaddr *)&server, len);
	if (rc < 0) {
		std::cerr << "Error! bind failed." << std::endl;
		return 0;
	}

	rc = listen(s, SOMAXCONN);
	if (rc < 0) {
		std::cerr << "Error! listen failed." << std::endl;
		return 0;
	}

	for(;;) {
		memset(&client, 0, len);
		c = accept(s, (struct sockaddr *)&client, &len);
		if (c < 0) {
			std::cerr << "Error! accept failed." << std::endl;
			return 0;
		}
		TalkToClient(c);
		//close(c);
	}

}

		
