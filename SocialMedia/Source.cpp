#define _CRT_SECURE_NO_WARNINGS
#define USERS_LIMIT 256
#define USER_STRUCT_SIZE 64
#define USER_START 1024
#define UNAME_SIZE 30
#define PWORD_SIZE 30
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define RQ_TYPE_REG 1
#define RQ_TYPE_LOGIN 2
#define RQ_TYPE_SEND_MSG 3
#define RQ_TYPE_GET_ALL_MSGS 4
#define CREATE_POST 5
#define VIEW_ALL_POSTS 6
#define VIEW_ALL_POSTS_BY_ID 7
#define VIEW_ALL_COMMENTS 8

#define DB_FILE_NAME "db_file.bin"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib")


FILE* db_fptr;

typedef struct meta_data
{
	int no_users;
	int post_data_offset;
	int comment_offset;
	int messages_offset;
	char unused[1008];
}meta_data;

typedef struct user
{
	char uname[UNAME_SIZE];
	char pword[PWORD_SIZE];
	int key;
}user;


void init_database()
{
	meta_data* _meta_data = (meta_data*)malloc(1 * sizeof(meta_data));
	_meta_data->no_users = 0;
	_meta_data->post_data_offset = 0;
	_meta_data->comment_offset = 0;
	_meta_data->messages_offset = 0;
	fseek(db_fptr, 0, SEEK_SET);
	fwrite((void*)_meta_data, sizeof(meta_data), 1, db_fptr);
	fseek(db_fptr, 1024 * 1024 * 100 - 1, SEEK_SET);
	fwrite("1", 1, 1, db_fptr);
	return;
}

void create_user(char *uname, char *pword)
{
	long offset = 0;

	user* _user = NULL;

	meta_data* _meta_data = (meta_data*)malloc(1 * sizeof(meta_data));
	fseek(db_fptr, 0, SEEK_SET);
	fread((void*)_meta_data, sizeof(meta_data), 1, db_fptr);

	if (_meta_data->no_users >= USERS_LIMIT)
	{
		printf("No of users limit crossed!\n");
		return;
	}
	else
	{
		_user = (user*)calloc(1, sizeof(user));
		_user->key = _meta_data->no_users;
		strcpy(_user->uname, uname);
		strcpy(_user->pword, pword);

		//printf("%s-%s-%d\n", uname, pword, _user->key);

		offset = (_meta_data->no_users)*USER_STRUCT_SIZE + USER_START;
		fseek(db_fptr, offset, SEEK_SET);
		fwrite((void*)_user, sizeof(user), 1, db_fptr);
		free(_user);

		fseek(db_fptr, 0, SEEK_SET);
		_meta_data->no_users = (_meta_data->no_users) + 1;
		fwrite((void*)_meta_data, sizeof(meta_data), 1, db_fptr);
		free(_meta_data);
	}
	return;
}

int get_user_key(char* uname)
{
	meta_data* _meta_data = (meta_data*)calloc(1, sizeof(meta_data));
	user* _user = NULL;
	long offset;

	fseek(db_fptr, 0, SEEK_SET);
	fread((void*)_meta_data, sizeof(meta_data), 1, db_fptr);


	int key = -1;
	for (size_t i = 0; i < unsigned(_meta_data->no_users); i++)
	{
		_user = (user*)calloc(1, sizeof(user));
		offset = (i * USER_STRUCT_SIZE) + USER_START;
		fseek(db_fptr, offset, SEEK_SET);
		fread((void*)_user, sizeof(user), 1, db_fptr);

		if (strcmp(_user->uname, uname) == 0)
		{
			return _user->key;
		}
	}
	return key;
}

char* get_pword(char* uname)
{
	int key = get_user_key(uname);
	if (key == -1)
	{
		return NULL;
	}

	user* _user = (user*)calloc(1, sizeof(user));
	char* pword = (char*)calloc(PWORD_SIZE, sizeof(char));

	long offset = key * USER_STRUCT_SIZE + USER_START;

	fseek(db_fptr, offset, SEEK_SET);

	fread((void*)_user, sizeof(user), 1, db_fptr);


	strcpy(pword, _user->pword);

	free(_user);

	return pword;
}


void handle_register(SOCKET s){
	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);

	int size;
	char* reply = (char*)malloc(60);

	if ((size = recv(s, reply, 60, 0)) > 0)
	{
		char* username = (char*)malloc(UNAME_SIZE);
		char* password = (char*)malloc(PWORD_SIZE);

		for (int i = 0; i < UNAME_SIZE; i++)
		{
			username[i] = reply[i];
			password[i] = reply[UNAME_SIZE + i];
		}
		create_user(username, password);

		char* message = "SUCCESS!";
		send(s, message, strlen(message), 0);
	}
}

void handle_login(SOCKET s)
{
	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);
	
	int size;
	char* reply = (char*)malloc(60);

	if ((size = recv(s, reply, 60, 0)) > 0)
	{
		char* username = (char*)malloc(UNAME_SIZE);
		char* password = (char*)malloc(PWORD_SIZE);

		for (int i = 0; i < UNAME_SIZE; i++)
		{
			username[i] = reply[i];
			password[i] = reply[UNAME_SIZE + i];
		}
		char* a_password = get_pword(username);
		if (a_password == NULL){
			char* message = "USER NOT REGISTERED!";
			send(s, message, strlen(message), 0);
		}
		if (strcmp(a_password, password) == 0)
		{
			char* message = "SUCCESS!";
			send(s, message, strlen(message), 0);
		}
		else
		{
			char* message = "FAIL!";
			send(s, message, strlen(message), 0);
		}
	}
}

void handle_send_message(SOCKET s){

}

void handle_get_all_messages(SOCKET s){

}

void handle_create_post(SOCKET s){

}

void handle_view_all_posts(SOCKET s){

}

void handle_view_user_posts(SOCKET s){

}

void handle_requests(SOCKET s,int type, int size)
{
	switch (type)
	{
	case 1:
		handle_register(s);
		break;
	
	case 2:
		handle_login(s);
		break;
		
	case 3:
		handle_send_message(s);
		break;

	case 4:
		handle_get_all_messages(s);
		break;

	case 5:
		handle_create_post(s);
		break;

	case 6:
		handle_view_all_posts(s);
		break;

	case 7:
		handle_view_user_posts(s);
		break;

	default:
		printf("Wrong request type!\n");
	}
}

void accept_request(SOCKET s)
{
	int c = sizeof(struct sockaddr_in);
	struct sockaddr_in client;

	SOCKET s1;
	s1 = accept(s, (sockaddr*)&client, &c);
	if (s1 == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}

	puts("Connection accepted");

	int recvsize = 0;
	size_t size;
	int type;
	int reply[2];
	if ((recvsize = recv(s1, (char*)reply, 2, 0)) > 0)
	{
		type = reply[0];
		size = reply[1];
	}

}

int listen_to_client(SOCKET* s)
{
	WSADATA wsa;
	struct sockaddr_in server;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Intialization Successful\n");

	if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("\nCreated socket successfully\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	if (bind(*s, (const sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}

	puts("Bind done");

	listen(*s, 5);

	puts("Waiting for incoming connections... at ");
}

int main()
{
	db_fptr = fopen(DB_FILE_NAME, "rb+");
	if (db_fptr == NULL)
	{
		db_fptr = fopen(DB_FILE_NAME, "wb");
		fclose(db_fptr);
		db_fptr = fopen(DB_FILE_NAME, "rb+");
	}

	init_database();
	
	SOCKET s;

	if (listen_to_client(&s))
	{
		printf("Connection unsuccessful!\n");
		return 0;
	}


	return 0;

}