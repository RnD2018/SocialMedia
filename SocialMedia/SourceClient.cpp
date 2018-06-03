#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define UNAME_SIZE 30
#define PWORD_SIZE 30
#pragma comment(lib,"ws2_32.lib")



int connect_to_server(SOCKET *s)
{
	WSADATA wsa;
	struct sockaddr_in server;
	//char *message, server_reply[5000];
	//int recv_size;

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


	server.sin_addr.s_addr = inet_addr("192.168.43.169");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(*s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

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

//void _add_post();
//void _view_posts();
//void _view_my_posts();
//// menu after logging in.
//void menu_level_two()
//{
//	int option = 0;
//	int exit_flag = 0;
//	while (1)
//	{
//		switch (option)
//		{
//		case 1:
//			_add_post();
//			break;
//		case 2:
//			_view_posts();
//			break;
//		case 3:
//			_view_my_posts();
//			break;
//		default:
//			exit_flag = 1;
//			break;
//		}
//		if (exit_flag)
//		{
//			break;
//		}
//	}
//}

void _register(SOCKET s)
{
	char* uname = (char*)calloc(UNAME_SIZE, sizeof(char));
	char* pword = (char*)calloc(PWORD_SIZE, sizeof(char));
	char* buffer = (char*)calloc(UNAME_SIZE + PWORD_SIZE, sizeof(char));

	printf("Username : ");
	scanf("%[^\n]", uname);

	printf("Password : ");
	scanf("%[^\n]", pword);

	for (size_t i = 0; i < unsigned(UNAME_SIZE); i++)
	{
		buffer[i] = uname[i];
		buffer[i + UNAME_SIZE] = pword[i];
	}

	int req[2] = { 1, -1 };
	send(s, (char*)req, 8, 0);
	
	int size; 
	if ((size = recv(s, (char*)req, 8, 0)) > 0)
	{
		send(s, buffer, UNAME_SIZE + PWORD_SIZE, 0);
	}

	buffer = (char*)calloc(20, sizeof(char));
	if ((size = recv(s, buffer, 20, 0)) > 0)
	{
		if (strcmp(buffer, "SUCCESS!") == 0)
		{
			printf("SERVER REPLY : %s\n", buffer);
		}
		else
		{
			printf("SERVER REPLY : %s\n", buffer);
		}
	}
	
	return;
}

void _login(SOCKET s)
{
	char* uname = (char*)calloc(UNAME_SIZE, sizeof(char));
	char* pword = (char*)calloc(PWORD_SIZE, sizeof(char));
	char* buffer = (char*)calloc(UNAME_SIZE + PWORD_SIZE, sizeof(char));

	printf("Username : ");
	scanf("%[^\n]", uname);

	printf("Password : ");
	scanf("%[^\n]", pword);

	for (size_t i = 0; i < unsigned(UNAME_SIZE); i++)
	{
		buffer[i] = uname[i];
		buffer[i + UNAME_SIZE] = pword[i];
	}

	int req[2] = { 1, -1 };
	send(s, (char*)req, 8, 0);

	int size;
	if ((size = recv(s, (char*)req, 8, 0)) > 0)
	{
		send(s, buffer, UNAME_SIZE + PWORD_SIZE, 0);
	}

	buffer = (char*)calloc(100, sizeof(char));
	if ((size = recv(s, buffer, 100, 0)) > 0)
	{
		if (strcmp(buffer, "USER NOT REGISTERED!") == 0)
		{
			printf("SERVER REPLY : %s\n", buffer);
		}
		else if (strcmp(buffer, "SUCCESS!") == 0)
		{
			printf("SERVER REPLY : %s\n", buffer);
		}
		else
		{
			printf("SERVER REPLY : %s\n", buffer);
		}
	}

	return;

}



// initial startup menu.
void menu_level_one(SOCKET s)
{
	int option = 0;
	int exit_flag = 0;
	while (1)
	{
		printf("1. Register.\n");
		printf("2. Login.\n");
		printf(">>");
		scanf("%d", &option);

		switch (option)
		{
		case 1:
			_register(s);
			break;
		case 2:
			_login(s);
			break;
		default:
			exit_flag = 1;
			break;
		}

		if (exit_flag)
		{
			break;
		}

	}
}

int main()
{
	SOCKET s;
	if (connect_to_server(&s))
	{
		printf("Connection unsuccessful!\n");
		menu_level_one(s);
		return 0;
	}

	return 0;
}