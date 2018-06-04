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

#define POST_OFFSET 148480
#define COMMENT_OFFSET 17408
#define FILE_SIZE 523264
#define POST_META_DATA_OFFSET 4
#define COMMENT_META_DATA_OFFSET 8
#define MAX_SIZE 512
#define VIEW_ALL_POSTS 6
#define VIEW_ALL_POSTS_BY_ID 7
#define VIEW_ALL_COMMENTS 8
#define UNKNOWN_SIZE -1
#define MAX_POST_SIZE 524016
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

typedef struct message_info
{
	int sender_key;
	int receiver_key;
	int size;
}message_info;

struct PostDataClient
{
	char username[20];
	char post_description[204];
	char file_name[40];
	char post_content[MAX_POST_SIZE];
};

struct PostDataServer
{
	int post_id;
	int user_id;
	char username[20];
	char post_description[204];
	char file_name[40];
	char post_content[MAX_POST_SIZE];
};

//structure for posts
typedef struct post
{
	int post_id;
	char username[20];
	char post_desc[204];
	char filename[40];
	int likes;
} post;

//structure for comments
typedef struct comment
{
	int post_id;
	int user_id;
	char comment_string[120];
} comment;

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

int get_user_key(char*);


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
		fflush(db_fptr);
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
		if (get_user_key(username)!=-1){
			char* message = "USER EXISTS!";
			send(s, message, strlen(message), 0);
		}
		else
		{
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
		else if (strcmp(a_password, password) == 0)
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
struct _message{
	char sender[30];
	char receiver[30];
	char message[500];
};
/* breaks recieved message into usernames and message*/
char** break_message(struct _message* reply, int size)
{
	char** broken_message = (char**)malloc(3 * sizeof(char*));

	char* sender = (char*)calloc(30, 1);
	for (int i = 0; i < 30; i++)
	{
		sender[i] = reply->sender[i];
	}

	char* receiver = (char*)calloc(30, 1);
	for (int i = 0; i < 30; i++)
	{
		receiver[i] = reply->receiver[ i];
	}
	char* message = (char*)calloc(size - 59, 1);
	for (int i = 0; i < size - 60; i++)
	{
		message[i] = reply->message[i];
	}
	broken_message[0] = sender;
	broken_message[1] = receiver;
	broken_message[2] = message;
	return broken_message;
}

//typedef struct message_info{
//	int sender_key;
//	int receiver_key;
//	int size;
//}message_info;


void insert_message(char** broken_mesage, int size)
{
	char* sender = broken_mesage[0];
	char* receiver = broken_mesage[1];
	char* message = broken_mesage[2];

	int offset = get_messages_offset();

	int present_offset = offset + size - 60 + sizeof(message_info);
	fseek(db_fptr, -(present_offset + 4), SEEK_END);
	int check_size = -1;
	fwrite(&check_size, 4, 1, db_fptr);
	fwrite(message, size - 60, 1, db_fptr);

	message_info* info = (message_info*)malloc(sizeof(message_info));
	int  s_key = get_user_key(sender);
	int r_key = get_user_key(receiver);
	
	fwrite(&s_key, 4, 1, db_fptr);
	fwrite(&r_key, 4, 1, db_fptr);
	size -= 60;
	fwrite(&size, 4,1, db_fptr);
	info->sender_key = get_user_key(sender);
	info->receiver_key = get_user_key(receiver);
	info->size = size - 60;
	//fwrite(info, sizeof(message_info), 1, db_fptr);
	free(info);
	set_messages_offset(present_offset);
	fflush(db_fptr);
}


void receive_message(SOCKET s, int size)
{
	char* reply = (char*)malloc(size);
	int reply_size;
	struct _message* mesg = (struct _message*) malloc(sizeof(struct _message));
	if ((reply_size = recv(s, (char*)mesg, size + 1, 0)) > 0)
	{
		char** broken_message = break_message(mesg, size);
		insert_message(broken_message, size);

		char* message = "Message sent successfully!";
		send(s, message, strlen(message), 0);
	}
}
void handle_send_message(SOCKET s, int size){
	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);
	receive_message(s, size);
}


/* GET ALL MESSAGES */
typedef struct _s_message{
	int size;
	char sender[20];
	char* message;
}_s_message;
_s_message** extend_size(_s_message** user_messages, int* size){

	_s_message** new_user_messages = (_s_message**)malloc(2 * (*size)*sizeof(char*));
	for (int i = 0; i < *size; i++)
	{
		new_user_messages[i] = user_messages[i];
	}
	*size = 2 * (*size);
	return new_user_messages;
}
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


_s_message** get_user_messages(char* user_name, int* len, int* total_size){

	*len = 50;
	*total_size = 0;
	_s_message** user_messages = (_s_message**)malloc((*len)*sizeof(char*));
	int message_count = 0;

	int user_key = get_user_key(user_name);
	fseek(db_fptr, 0, SEEK_END);
	while (1)
	{
		//fseek(db_fptr, -12, SEEK_CUR);;

		fseek(db_fptr, -4, SEEK_END);
		int a;
		fread(&a, 4, 1, db_fptr);
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
	int response[2];
	response[0] = 1;
	response[1] = size_bytes + 4;
	send(s, (char*)response, 8, 0);

	char* message = get_message_string(messages, message_count, size_bytes);

	send(s, message, size_bytes + 4, 0);/*
										printf("Messages sent to user!\n");*/
}

void handle_get_all_messages(SOCKET s)
{
	
	char* username = (char*)calloc(30,1);
	if ((recv(s, username, 30, 0) > 0))
	{
		response_all_messages(s, username);
	}
}
void getPostFromClient(SOCKET s, int size)
{
	struct PostDataClient *post_data_from_client = (struct PostDataClient*)malloc(sizeof(struct PostDataClient));
	int recv_size;

	if ((recv_size = recv(s, (char*)post_data_from_client, size, 0)) > 0)
	{
		//post_data_from_client[recv_size] = '\0';
		printf("Debug: recieve size : posts : %d", recv_size);
	}

	struct PostDataServer *post_data_from_cli = (struct PostDataServer*)malloc(sizeof(struct PostDataServer));//(struct PostDataServer*)post_data_from_cl;
	int post_meta_data_offset;
	fseek(db_fptr, 4, SEEK_SET);//To read the post id

	fread(&post_meta_data_offset, sizeof(int), 1, db_fptr);//Reads the post number into the offset

	//post_meta_data_offset = 1;

	post_data_from_cli->post_id = post_meta_data_offset;

	post_data_from_cli->user_id = get_user_key(post_data_from_cli->username);

	strcpy(post_data_from_cli->post_description, post_data_from_client->post_description);

	strcpy(post_data_from_cli->post_content, post_data_from_client->post_content);

	strcpy(post_data_from_cli->username, post_data_from_client->username);

	strcpy(post_data_from_cli->file_name, post_data_from_client->file_name);

	fseek(db_fptr, (post_meta_data_offset * 512 * 1024) + 148480, SEEK_SET);

	fwrite(post_data_from_cli, sizeof(struct PostDataServer), 1, db_fptr);
	post_meta_data_offset++;
	fseek(db_fptr, 4, SEEK_SET);
	fwrite(&post_meta_data_offset, sizeof(int), 1, db_fptr);
	printf("Successfully posted\n");
	fflush(db_fptr);
	send(s, "Successfully uploaded", 21, 0);
}



void handle_create_post(SOCKET s, int size)
{
	int response[] = { 1, -1 };
	send(s, (char*)response, 8, 0);
	getPostFromClient(s, size);
}

//char *comment_to_string(comment comment_data)
//{
//	char *data = (char *)calloc(MAX_SIZE, sizeof(char));
//	char *temp = (char *)calloc(MAX_SIZE, sizeof(char));
//	if (((char *)&comment_data)[0] == -1)
//	{
//		data[0] = -1;
//		return data;
//	}
//	strcpy(data, comment_data.user_id);
//	strcat(data, "|");
//	strcat(data, comment_data.comment_string);
//	return data;
//}

//char *get_comments(int post_id)
//{
//	static int comm_no;
//	int no_of_comms;
//	comment comment_data;
//	char *comments = (char *)calloc(MAX_SIZE, sizeof(char));
//	fseek(db_fptr, COMMENT_META_DATA_OFFSET, SEEK_SET);
//	fread(&no_of_comms, sizeof(int), 1, db_fptr);
//	while (comm_no < no_of_comms)
//	{
//		fseek(db_fptr, COMMENT_OFFSET + (comm_no * sizeof(comment)), SEEK_SET);
//		fread(&comment_data, sizeof(comment), 1, db_fptr);
//		comm_no++;
//		if (post_id == comment_data.post_id) strcpy(comments, comment_to_string(comment_data));
//		else continue;
//	}
//	return comments;
//}

post get_posts_by_id(char *username)
{
	static int post_no = 0;
	int no_of_posts;
	post post_data;
	fseek(db_fptr, 4, SEEK_SET);
	fread(&no_of_posts, sizeof(post), 1, db_fptr);
	while (post_no < no_of_posts)
	{
		fseek(db_fptr, POST_OFFSET + (sizeof(PostDataServer) * post_no), SEEK_SET);
		fread(&post_data, sizeof(post), 1, db_fptr);
		post_no++;
		if (strcmp(post_data.username, ""))
			return post_data;
		else if (strcmp(post_data.username,username))
			return post_data;
		else
			continue;
	}
}

//get_comments(atoi(strtok(temp, "|")))

void send_all_posts_by_id(SOCKET s, int flag)
{
	char *username = (char *)calloc(30, sizeof(char));
	if (!flag)
		recv(s, username, 30, 0);
	else
		username = "";
	post *post_data = (post *)calloc(MAX_SIZE, sizeof(post));
	post temp;
	temp = get_posts_by_id(username);
	int i = 0;
	while (temp.post_id != -1)
	{
		post_data[i++] = temp;
	}
	int response[2] = { 1, i };
	send(s, (char *)response, 8, 0);
	send(s, (char *)post_data, sizeof(post) * i, 0);
}

void send_all_posts(SOCKET s)
{
	send_all_posts_by_id(s, 1);
}

void handle_view_all_posts(SOCKET s)
{
	send_all_posts(s);
}

void handle_view_user_posts(SOCKET s)
{
	int resposne[] = { 1, -1 };
	send(s, (char*)resposne, 8, 0);
	send_all_posts_by_id(s, 0);
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
		handle_create_post(s, size);
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
	while (true)
	{
		int recvsize = 0;
		size_t size;
		int type;
		int reply[2];
		if ((recvsize = recv(s1, (char*)reply, 8, 0)) > 0)
		{
			type = reply[0];
			size = reply[1];
		}
		handle_requests(s1, type, size);
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
		init_database();
		fclose(db_fptr);
		db_fptr = fopen(DB_FILE_NAME, "rb+");
		printf("DEBUG: DB : CREATED NEW ONE\n");
	}
	printf("DEBUG: DB : CONNECTED TO EXISTING ONE\n");
	
	SOCKET s;

	if (listen_to_client(&s))
	{
		printf("Connection unsuccessful!\n");
		return 0;
	}

	accept_request(s);

	return 0;

}
//
//void insert_message(char** broken_mesage, int size)
//{
//	char* sender = broken_mesage[0];
//	char* receiver = broken_mesage[1];
//	char* message = broken_mesage[2];
//
//	int offset = get_messages_offset();
//
//	int present_offset = offset + size - 60 + sizeof(message_info);
//	fseek(db_fptr, -(present_offset - 4), SEEK_END);
//	int check_size = -1;
//	fwrite(&check_size, 4, 1, db_fptr);
//	fwrite(message, size - 60, 1, db_fptr);
//
//	message_info* info = (message_info*)malloc(sizeof(message_info));
//	info->sender_key = get_user_key(sender);
//	info->receiver_key = get_user_key(receiver);
//	info->size = size - 60;
//	fwrite(info, sizeof(message_info), 1, db_fptr);
//	free(info);
//	set_messages_offset(present_offset);
//
//}
//
//void handle_send_message(SOCKET s,int size){
//
//	int response[] = { 1, -1 };
//	send(s, (char*)response, 8, 0);
//
//	char* reply = (char*)malloc(size);
//	int reply_size;
//
//	if ((reply_size = recv(s, reply, size + 1, 0)) > 0)
//	{
//		char** broken_message = break_message(reply, size);
//		insert_message(broken_message, size);
//	}
//
//}
//
//
//
///* GET ALL MESSAGES */
///* Structure of the message to be sent from server, for get all messages from user*/
//typedef struct _s_message{
//	int size;
//	char sender[30];
//	char* message;
//}_s_message;
//
//char* get_uname(int key)
//{
//	if (key < 0 || key > 255)
//	{
//		return NULL;
//	}
//
//	char* uname = (char*)calloc(UNAME_SIZE, sizeof(char));
//	user* _user = (user*)calloc(1, sizeof(user));
//	long offset = key*USER_STRUCT_SIZE + USER_START;
//
//	fseek(db_fptr, offset, SEEK_SET);
//	fread((void*)_user, sizeof(user), 1, db_fptr);
//
//	strcpy(uname, _user->uname);
//
//	return uname;
//}
//
///* Helper function to extend size of the pointer if exceeded.*/
//_s_message** extend_size(_s_message** user_messages, int* size){
//
//	_s_message** new_user_messages = (_s_message**)malloc(2 * (*size)*sizeof(char*));
//	for (int i = 0; i < *size; i++)
//	{
//		new_user_messages[i] = user_messages[i];
//	}
//	*size = 2 * (*size);
//	return new_user_messages;
//}
//
//_s_message** get_user_messages(char* user_name, int* len, int* total_size){
//
//	*len = 50;
//	*total_size = 0;
//	_s_message** user_messages = (_s_message**)malloc((*len)*sizeof(char*));
//	int message_count = 0;
//
//	int user_key = get_user_key(user_name);
//	fseek(db_fptr, 0, SEEK_END);
//	while (1)
//	{
//		fseek(db_fptr, -12, SEEK_CUR);
//		message_info* info = (message_info*)malloc(sizeof(message_info));
//		fread(info, 12, 1, db_fptr);
//		if (info->size == -1)
//			break;
//
//		fseek(db_fptr, -(info->size + 12), SEEK_CUR);
//
//		if (info->receiver_key == user_key)
//		{
//			char* message = (char*)calloc(info->size + 1, 1);
//			fread(message, info->size, 1, db_fptr);
//			fseek(db_fptr, -info->size, SEEK_CUR);
//			if (message_count >= *len)
//				extend_size(user_messages, len);
//
//			_s_message* reply = (_s_message*)malloc(sizeof(_s_message));
//			reply->message = message;
//			strcpy(reply->sender, get_uname(info->sender_key));
//			reply->size = strlen(message);
//
//			user_messages[message_count++] = reply;
//			*total_size = *total_size + strlen(message);
//		}
//	}
//	*len = message_count;
//
//	return user_messages;
//}
//
//char* get_message_string(_s_message** messages, int message_count, int bytes_size)
//{
//
//	char* message = (char*)calloc(bytes_size + 4, 1);
//	int index = 0;
//	for (int i = 0; i < message_count; i++)
//	{
//		int size = messages[i]->size;
//		*((int*)message + index) = size;
//		index += 4;
//		for (int i = 0; i < 30; i++)
//		{
//			message[index + i] = messages[i]->sender[i];
//		}
//		index += 30;
//		for (int i = 0; i < size; i++)
//		{
//			message[index + i] = messages[i]->message[i];
//		}
//		index += size;
//	}
//	*((int*)message + index) = -1;
//	return message;
//}
//
//void response_all_messages(SOCKET s, char* user_name)
//{
//	int message_count;
//	int size_bytes;
//	_s_message** messages = get_user_messages(user_name, &message_count, &size_bytes);
//
//	size_bytes += (30 + 4)*message_count;
//	char* message = get_message_string(messages, message_count, size_bytes);
//
//	send(s, message, size_bytes + 4, 0);
//	printf("Messages sent to user!\n");
//}
//void handle_get_all_messages(SOCKET s){
//	int response[] = { 1, -1 };
//	send(s, (char*)response, 8, 0);
//	char* reply = (char*)malloc(30);
//	int size;
//	if ((size = recv(s, reply, 30, 0)) > 0)
//	{
//		response_all_messages(s, reply);
//	}
//	
//}
//
//void handle_create_post(SOCKET s){
//
//}
//
//void handle_view_all_posts(SOCKET s){
//
//}
//
//void handle_view_user_posts(SOCKET s){
//
//}
//
//void handle_requests(SOCKET s,int type, int size)
//{
//	switch (type)
//	{
//	case 1:
//		handle_register(s);
//		break;
//	
//	case 2:
//		handle_login(s);
//		break;
//		
//	case 3:
//		handle_send_message(s,size);
//		break;
//
//	case 4:
//		handle_get_all_messages(s);
//		break;
//
//	case 5:
//		handle_create_post(s);
//		break;
//
//	case 6:
//		handle_view_all_posts(s);
//		break;
//
//	case 7:
//		handle_view_user_posts(s);
//		break;
//
//	default:
//		printf("Wrong request type!\n");
//	}
//}
//
//void accept_request(SOCKET s)
//{
//	int c = sizeof(struct sockaddr_in);
//	struct sockaddr_in client;
//
//	SOCKET s1;
//	s1 = accept(s, (sockaddr*)&client, &c);
//	if (s1 == INVALID_SOCKET)
//	{
//		printf("accept failed with error code : %d", WSAGetLastError());
//	}
//
//	puts("Connection accepted");
//
//	int recvsize = 0;
//	size_t size;
//	int type;
//	int reply[2];
//	if ((recvsize = recv(s1, (char*)reply, 2, 0)) > 0)
//	{
//		type = reply[0];
//		size = reply[1];
//	}
//
//}
//
//int listen_to_client(SOCKET* s)
//{
//	WSADATA wsa;
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
//	if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
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
//	if (bind(*s, (const sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
//	{
//		printf("Bind failed with error code : %d", WSAGetLastError());
//	}
//
//	puts("Bind done");
//
//	listen(*s, 5);
//
//	puts("Waiting for incoming connections... at ");
//}
//
//int main()
//{
//	db_fptr = fopen(DB_FILE_NAME, "rb+");
//	if (db_fptr == NULL)
//	{
//		db_fptr = fopen(DB_FILE_NAME, "wb");
//		fclose(db_fptr);
//		db_fptr = fopen(DB_FILE_NAME, "rb+");
//	}
//
//	init_database();
//	
//	SOCKET s;
//
//	if (listen_to_client(&s))
//	{
//		printf("Connection unsuccessful!\n");
//		return 0;
//	}
//
//	accept_request(s);
//
//	return 0;
//
//}
