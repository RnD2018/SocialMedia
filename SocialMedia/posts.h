#include <stdio.h>
#include <string.h>
#include<winsock2.h>
#define POST_OFFSET 148480
#define COMMENT_OFFSET 17408
#define FILE_SIZE 523264
#define DB_FILE_NAME "db_file.bin"
#define POST_META_DATA_OFFSET 4
#define COMMENT_META_DATA_OFFSET 8
#define MAX_SIZE 512
#define VIEW_ALL_POSTS 6
#define VIEW_ALL_POSTS_BY_ID 7
#define VIEW_ALL_COMMENTS 8
#define UNKNOWN_SIZE -1

#pragma comment(lib,"ws2_32.lib") //Winsock Library

//structure for posts
typedef struct post
{
	int post_id;
	int user_id;
	char post_desc[204];
	char filename[40];
	int likes;
	char file[FILE_SIZE];
} post;

//structure for comments
typedef struct comment
{
	int post_id;
	int user_id;
	char comment_string[120];
} comment;

typedef struct request
{
	int type;
	int size;
	char username_self[30];
	char username_others[30];
} request;

//File handler for database
FILE *db_fptr = fopen(DB_FILE_NAME, "rb+");

////////////////CLIENT/////////////////////

SOCKET *s;

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

//void send_request_to_server(SOCKET *s, void *request)
//{
//	char req_type_size[2], server_reply[MAX_SIZE];
//	req_type_size[0] = req_type;
//	req_type_size[1] = size;
//	int recv_size;
//	send(*s, req_type_size, 2, 0);
//	//Receive a reply from the server
//	if ((recv_size = recv(*s, server_reply, 2, 0)) > 0 && strcmp(server_reply, "OK"))
//	{
//		server_reply[recv_size] = '\0';
//		printf("Server reply: %s\n", server_reply);
//	}
//	while (server_reply[0] != -1 && (recv_size = recv(*s, server_reply, 2, 0)) > 0)
//	{
//		server_reply[recv_size] = '\0';
//		printf("%s\n", server_reply);
//	}
//}

void close_connection(SOCKET *s)
{
	puts("Closing connection to server\n");
	closesocket(*s);
	WSACleanup();
	puts("Connection closed to the server\n");
}

void parse_and_print_comments(char *reply)
{
	char *token = (char *)calloc(MAX_SIZE, sizeof(char));
	token = strtok(reply, "|");
	printf("\t%s's comment: ", token);
	token = strtok(NULL, "|");
	printf("%s\n", token);
}

void get_all_comments_for_post(int post_id)
{
	char server_reply[MAX_SIZE];
	int recv_size;
	int req_type_size[2] = { VIEW_ALL_COMMENTS, UNKNOWN_SIZE };
	send(*s, (char *)req_type_size, 2 * sizeof(int), 0);
	//Receive a reply from the server
	if ((recv_size = recv(*s, server_reply, 2, 0)) > 0 && strcmp(server_reply, "OK"))
	{
		server_reply[recv_size] = '\0';
		printf("Server reply: %s\n", server_reply);
	}
	while (server_reply[0] != -1 && (recv_size = recv(*s, server_reply, 2, 0)) > 0)
	{
		server_reply[recv_size] = '\0';
		parse_and_print_comments(server_reply);
	}
}

void parse_and_print_post(char *reply)
{
	char *token = (char *)calloc(MAX_SIZE, sizeof(char));
	token = strtok(reply, "|");
	int post_id = atoi(token);
	printf("Post ID: %d\t", post_id);
	token = strtok(NULL, "|");
	printf("Posted by: %s\n", token);
	token = strtok(NULL, "|");
	printf("Post description: %d\n", token);
	token = strtok(NULL, "|");
	printf("Post filename: %s\n", token);
	FILE *temp = fopen(token, "wb");
	token = strtok(NULL, "|");
	fwrite(token, sizeof(char), strlen(token), temp);
	fclose(temp);
	token = strtok(NULL, "|");
	printf("Likes : %d\n", atoi(token));
	printf("Comments :\n");
	get_all_comments_for_post(post_id);
}

/*
Gets all posts created by a user(based on given username)
params:
	username : string indicating the username of the user
returns:
	nothing
*/
void recieve_all_posts_by_id(char *username)
{
	char server_reply[MAX_SIZE];
	int recv_size;
	int req_type_size[2] = { VIEW_ALL_POSTS, UNKNOWN_SIZE };
	send(*s, (char *)req_type_size, 2 * sizeof(int), 0);
	printf("Sent request to server to get all posts!\n");
	//Receive a reply from the server
	if ((recv_size = recv(*s, server_reply, 2, 0)) > 0 && !strcmp(server_reply, "OK"))
	{
		server_reply[recv_size] = '\0';
		printf("Server reply: %s\n", server_reply);
	}
	send(*s, username, strlen(username), 0);
	printf("All posts - %s\n", username);
	while (server_reply[0] != EOF && (recv_size = recv(*s, server_reply, 2, 0)) > 0)
	{
		server_reply[recv_size] = '\0';
		parse_and_print_post(server_reply);
	}
}

/*
Gets all posts created by any user
params:
	nothing
returns:
	nothing
Prints all posts in feed
*/
void recieve_all_posts()
{
	recieve_all_posts_by_id("");
	//send_request_to_server(s, VIEW_ALL_POSTS, UNKNOWN_SIZE);
}


/////////////////SERVER///////////////////

int get_user_id(char *);
char *get_uname(int id);

int listen_to_client()
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

char *post_to_string(post post_data)
{
	char *data = (char *)calloc(MAX_SIZE, sizeof(char));
	char *temp = (char *)calloc(MAX_SIZE, sizeof(char));
	if (((char *)&post_data)[0] == -1)
	{
		data[0] = -1;
		return data;
	}
	itoa(post_data.post_id, temp, 10);
	strcpy(data, temp);
	strcat(data, "|");
	temp = get_uname(post_data.user_id);
	strcat(data, temp);
	strcat(data, "|");
	strcat(data, post_data.post_desc);
	strcat(data, "|");
	strcat(data, post_data.filename);
	strcat(data, "|");
	itoa(post_data.likes, temp, 10);
	strcat(data, temp);
	strcat(data, "|");
}

char *get_posts_by_id(int id)
{
	static int post_no = 0;
	int no_of_posts;
	post post_data;
	fseek(db_fptr, 4, SEEK_SET);
	fread(&no_of_posts, sizeof(post), 1, db_fptr);
	while (true)
	{
		fseek(db_fptr, POST_OFFSET + (sizeof(post) * post_no), SEEK_SET);
		fread(&post_data, sizeof(post), 1, db_fptr);
		post_no++;
		if (id == -1)
			return post_to_string(post_data);
		else if (post_data.user_id == id)
			return post_to_string(post_data);
		else
			continue;
	}
}

char *comment_to_string(comment comment_data)
{
	char *data = (char *)calloc(MAX_SIZE, sizeof(char));
	char *temp = (char *)calloc(MAX_SIZE, sizeof(char));
	if (((char *)&comment_data)[0] == -1)
	{
		data[0] = -1;
		return data;
	}
	strcpy(data, get_uname(comment_data.user_id));
	strcat(data, "|");
	strcat(data, comment_data.comment_string);
	return data;
}

char *get_comments(int post_id)
{
	static int comm_no;
	int no_of_comms;
	comment comment_data;
	fseek(db_fptr, COMMENT_META_DATA_OFFSET, SEEK_SET);
	fread(&no_of_comms, sizeof(int), 1, db_fptr);
	while (comm_no < no_of_comms)
	{
		fseek(db_fptr, COMMENT_OFFSET + (comm_no * sizeof(comment)), SEEK_SET);
		fread(&comment_data, sizeof(comment), 1, db_fptr);
		comm_no++;
		if (post_id == comment_data.post_id) comment_to_string(comment_data);
		else continue;
	}
}

void send_all_posts_by_id(int flag)
{
	int c = sizeof(struct sockaddr_in);
	int id;
	struct sockaddr_in client;
	SOCKET s1;
	s1 = accept(*s, (sockaddr *)&client, &c);
	if (s1 == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}

	puts("Connection accepted");
	char buffer[100];
	char reply[100];
	int recvsize = 0;
	size_t size;
	int type;
	
	if ((recvsize = recv(s1, reply, 2, 0)) > 0)
	{
		type = ((int *)reply)[0];
		size = ((int *)reply)[1];
	}
	send(s1, "OK", 2, 0);
	char *username = (char *)calloc(30, sizeof(char));
	if (!flag)
		recv(s1, username, 30, 0);
	else
		username = "";
	char *post_data = (char *)calloc(MAX_SIZE, sizeof(char));
	post_data = get_posts_by_id(id);
	while (post_data[0] == -1)
	{
		post_data = get_posts_by_id(id);
		if (post_data[0] == -1)
		{
			send(s1, &post_data[0], 1, 0);
			break;
		}
		send(s1, post_data, strlen(post_data), 0);
		post_data = get_comments(atoi(strtok(post_data, "|")));
		if (post_data[0] == -1)
		{
			send(s1, &post_data[0], 1, 0);
			continue;
		}
	}
}

void send_all_posts()
{
	send_all_posts_by_id(1);
}