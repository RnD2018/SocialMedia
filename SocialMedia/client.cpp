#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char *message, server_reply[5000];
	int recv_size;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");


	//server.sin_addr.s_addr = inet_addr("192.168.137.1");
	server.sin_addr.s_addr = inet_addr("192.168.43.80");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
	
		puts("connect error");
		return 1;
	}

	char buffer[100];
	puts("Connected");
	while (1){
		printf("\nEnter message: ");
		scanf("%s", buffer);
		send(s, buffer, strlen(buffer), 0);
		//Receive a reply from the server
		if ((recv_size = recv(s, server_reply, 2000, 0)) > 0)
		{
			server_reply[recv_size] = '\0';
			printf("%s\n", server_reply);
		}
	}

	puts("Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);


	closesocket(s);
	WSACleanup();

	return 0;
}