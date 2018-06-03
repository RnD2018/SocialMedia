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

#define MESSAGE_META 12

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
	fwrite("-1", 1, 1, db_fptr);
	return;
}


/* REGISTER */
int create_user(char *uname, char *pword)
{
	long offset = 0;

	user* _user = NULL;

	meta_data* _meta_data = (meta_data*)malloc(1 * sizeof(meta_data));
	fseek(db_fptr, 0, SEEK_SET);
	fread((void*)_meta_data, sizeof(meta_data), 1, db_fptr);

	if (_meta_data->no_users >= USERS_LIMIT)
	{
		printf("No of users limit crossed!\n");
		return -1;
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
	return 1;
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
		if (create_user(username, password))
		{
			char* message = "SUCCESS!";
			send(s, message, strlen(message), 0);
		}
		else
		{
			char* message = "LIMIT EXPIRED!";
			send(s, message, strlen(message), 0);
		}
		
	}
}



/* LOGIN */
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



/* SEND MESSAGE */
int get_messages_offset()
{
	fseek(db_fptr, MESSAGE_META, SEEK_SET);
	int offset;
	fread(&offset, 4, 1, db_fptr);
	return offset;
}

void set_messages_offset(int offset)
{
	fseek(db_fptr, MESSAGE_META, SEEK_SET);
	fwrite(&offset, 4, 1, db_fptr);
}

/* breaks recieved message into usernames and message*/
char** break_message(char* reply, int size)
{
	char** broken_message = (char**)malloc(3 * sizeof(char*));

	char* sender = (char*)calloc(30, 1);
	for (int i = 0; i < 30; i++)
	{
		sender[i] = reply[i];
	}

	char* receiver = (char*)calloc(30, 1);
	for (int i = 0; i < 30; i++)
	{
		receiver[i] = reply[30 + i];
	}
	char* message = (char*)calloc(size - 59, 1);
	for (int i = 0; i < size - 60; i++)
	{
		message[i] = reply[60 + i];
	}
	broken_message[0] = sender;
	broken_message[1] = receiver;
	broken_message[2] = message;
	return broken_message;
}
typedef struct message_info{
	int sender_key;
	int receiver_key;
	int size;
}message_info;

void insert_message(char** broken_mesage, int size)
{
	char* sender = broken_mesage[0];
	char* receiver = broken_mesage[1];
	char* message = broken_mesage[2];

	int offset = get_messages_offset();

	int present_offset = offset + size - 60 + sizeof(message_info);
	fseek(db_fptr, -(present_offset - 4), SEEK_END);
	int check_size = -1;
	fwrite(&check_size, 4, 1, db_fptr);
	fwrite(message, size - 60, 1, db_fptr);

	message_info* info = (message_info*)malloc(sizeof(message_info));
	info->sender_key = get_user_key(sender);
	info->receiver_key = get_user_key(receiver);
	info->size = size - 60;
	fwrite(info, sizeof(message_info), 1, db_fptr);
	free(info);
	set_messages_offset(present_offset);

}

void handle_send_message(SOCKET s,int size){

	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);

	char* reply = (char*)malloc(size);
	int reply_size;

	if ((reply_size = recv(s, reply, size + 1, 0)) > 0)
	{
		char** broken_message = break_message(reply, size);
		insert_message(broken_message, size);
	}

}



/* GET ALL MESSAGES */
/* Structure of the message to be sent from server, for get all messages from user*/
typedef struct _s_message{
	int size;
	char sender[30];
	char* message;
}_s_message;

char* get_uname(int key)
{
	if (key < 0 || key > 255)
	{
		return NULL;
	}

	char* uname = (char*)calloc(UNAME_SIZE, sizeof(char));
	user* _user = (user*)calloc(1, sizeof(user));
	long offset = key*USER_STRUCT_SIZE + USER_START;

	fseek(db_fptr, offset, SEEK_SET);
	fread((void*)_user, sizeof(user), 1, db_fptr);

	strcpy(uname, _user->uname);

	return uname;
}

/* Helper function to extend size of the pointer if exceeded.*/
_s_message** extend_size(_s_message** user_messages, int* size){

	_s_message** new_user_messages = (_s_message**)malloc(2 * (*size)*sizeof(char*));
	for (int i = 0; i < *size; i++)
	{
		new_user_messages[i] = user_messages[i];
	}
	*size = 2 * (*size);
	return new_user_messages;
}

_s_message** get_user_messages(char* user_name, int* len, int* total_size){

	*len = 50;
	*total_size = 0;
	_s_message** user_messages = (_s_message**)malloc((*len)*sizeof(char*));
	int message_count = 0;

	int user_key = get_user_key(user_name);
	fseek(db_fptr, 0, SEEK_END);
	while (1)
	{
		fseek(db_fptr, -12, SEEK_CUR);
		message_info* info = (message_info*)malloc(sizeof(message_info));
		fread(info, 12, 1, db_fptr);
		if (info->size == -1)
			break;

		fseek(db_fptr, -(info->size + 12), SEEK_CUR);

		if (info->receiver_key == user_key)
		{
			char* message = (char*)calloc(info->size + 1, 1);
			fread(message, info->size, 1, db_fptr);
			fseek(db_fptr, -info->size, SEEK_CUR);
			if (message_count >= *len)
				extend_size(user_messages, len);

			_s_message* reply = (_s_message*)malloc(sizeof(_s_message));
			reply->message = message;
			strcpy(reply->sender, get_uname(info->sender_key));
			reply->size = strlen(message);

			user_messages[message_count++] = reply;
			*total_size = *total_size + strlen(message);
		}
	}
	*len = message_count;

	return user_messages;
}

char* get_message_string(_s_message** messages, int message_count, int bytes_size)
{

	char* message = (char*)calloc(bytes_size + 4, 1);
	int index = 0;
	for (int i = 0; i < message_count; i++)
	{
		int size = messages[i]->size;
		*((int*)message + index) = size;
		index += 4;
		for (int i = 0; i < 30; i++)
		{
			message[index + i] = messages[i]->sender[i];
		}
		index += 30;
		for (int i = 0; i < size; i++)
		{
			message[index + i] = messages[i]->message[i];
		}
		index += size;
	}
	*((int*)message + index) = -1;
	return message;
}

void response_all_messages(SOCKET s, char* user_name)
{
	int message_count;
	int size_bytes;
	_s_message** messages = get_user_messages(user_name, &message_count, &size_bytes);

	size_bytes += (30 + 4)*message_count;
	char* message = get_message_string(messages, message_count, size_bytes);

	send(s, message, size_bytes + 4, 0);
	printf("Messages sent to user!\n");
}
void handle_get_all_messages(SOCKET s){
	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);
	char* reply = (char*)malloc(30);
	int size;
	if ((size = recv(s, reply, 30, 0)) > 0)
	{
		response_all_messages(s, reply);
	}
	
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
		handle_send_message(s,size);
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

	accept_request(s);

	return 0;

}