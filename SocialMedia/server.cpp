//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
//
//#include<stdio.h>
//#include<winsock2.h>
////#include<pthread.h>
//
//#pragma comment(lib,"ws2_32.lib")
//
//int main()
//{
//	WSADATA wsa;
//	SOCKET s;
//	struct sockaddr_in server;
//
//	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//	{
//		printf("Failed. Error Code : %d", WSAGetLastError());
//		return 1;
//	}
//
//	printf("Intialization Successful\n");
//
//	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
//	{
//		printf("Could not create socket : %d", WSAGetLastError());
//	}
//
//	printf("\nCreated socket successfully\n");
//
//	server.sin_family = AF_INET;
//	server.sin_addr.s_addr = INADDR_ANY;
//	server.sin_port = htons(8888);
//
//	if (bind(s, &server, sizeof(server)) == SOCKET_ERROR)
//	{
//		printf("Bind failed with error code : %d", WSAGetLastError());
//	}
//
//	puts("Bind done");
//
//	listen(s, 3);
//
//	puts("Waiting for incoming connections... at ");
//
//	int c = sizeof(struct sockaddr_in);
//	struct sockaddr_in client;
//	//struct sockaddr client;
//	SOCKET s1;
//	s1 = accept(s, &client, &c);
//	if (s1 == INVALID_SOCKET)
//	{
//		printf("accept failed with error code : %d", WSAGetLastError());
//	}
//
//	puts("Connection accepted");
//	char buffer[100];
//	char reply[100];
//	size_t size;
//	while (1)
//	{
//		printf("\nEnter message: ");
//		scanf("%s", buffer);
//		send(s1, buffer, strlen(buffer), 0);
//
//		if ((size = recv(s1, reply, 100, 0)) > 0)
//		{
//			reply[size] = '\0';
//			printf("Client: %s\n", reply);
//		}
//	}
//
//
//	getchar();
//
//	closesocket(s);
//	WSACleanup();
//	return 0;
//
//}