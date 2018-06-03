#include <stdio.h>
#include "posts.h"

int connect_to_server(SOCKET *s)
{
	WSADATA wsa;
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
	if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");


	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(*s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	char buffer[100];
	puts("Connected");
	return 0;
}

void close_connection(SOCKET *s)
{
	puts("Closing connection to server\n");
	closesocket(*s);
	WSACleanup();
	puts("Connection closed to the server\n");
}


int main()
{
	SOCKET s;
	if (connect_to_server(&s))
	{
		printf("Connection unsuccessful!\n");
		return 0;
	}

	return 0;
}