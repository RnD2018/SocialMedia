#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_POST_SIZE 511*1024+760
#define POST_START_POINTER 148480


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


//148480-MyOffset (num*512*1024)+MyOffset
//db_fptr


FILE *db_ptr;
char *current_user;
int num_of_bytes_in_file;

struct PostDataClient
{
	char username[20];
	char post_description[204];
	char post_content[MAX_POST_SIZE];
};



struct PostDataServer
{
	int post_id;
	int user_id;
	char username[20];
	char post_description[204];
	char post_content[MAX_POST_SIZE];
};




int getUserId(char username[])
{
	return 1;
}

void getPostFromClient(SOCKET s)
{
	int recv_size;
	char client_msg[5];
	int post_meta_data_offset;


	struct PostDataClient *post_data_from_client = (struct PostDataClient*)malloc(sizeof(struct PostDataClient));
	if ((recv_size = recv(s, client_msg, 5, 0)) > 0)
	{
		client_msg[recv_size] = '\0';
	}

	int *server_recv = (int*)malloc(sizeof(int)*2);
	server_recv = (int*)client_msg;

	char *msg;
	if (server_recv[0] == 5)//5 is Code for post
	{
		if (server_recv[1] < MAX_POST_SIZE)
		{
			msg = "ok";
			send(s, msg, strlen(msg), 0);
		}
		else
		{
			msg = "error";
			send(s, msg, strlen(msg), 0);
		}

		if ((recv_size = recv(s, (char*)post_data_from_client, sizeof(struct PostDataClient), 0)) > 0)
		{
			//post_data_from_client[recv_size] = '\0';
		}
		
		struct PostDataServer *post_data_from_cli = (struct PostDataServer*)malloc(sizeof(struct PostDataServer));//(struct PostDataServer*)post_data_from_cl;

		fseek(db_ptr, 4, SEEK_SET);//To read the post id

		fread(&post_meta_data_offset, sizeof(int), 1, db_ptr);//Reads the post number into the offset

		//post_meta_data_offset = 1;

		post_data_from_cli->post_id = post_meta_data_offset;

		post_data_from_cli->user_id = 1;//getUserId(post_data_from_cl->username);

		strcpy(post_data_from_cli->post_description, post_data_from_client->post_description);


		strcpy(post_data_from_cli->post_content, post_data_from_client->post_content);

		strcpy(post_data_from_cli->username, post_data_from_client->username);

		fseek(db_ptr, (post_meta_data_offset * 512 * 1024) + 148480, SEEK_SET);

		fwrite(post_data_from_cli, sizeof(struct PostDataServer), 1, db_ptr);

	}

}



char* getFileData(char *filename)
{
	FILE *fp = fopen(filename, "rb");
	char file_data[511*1024];
	int file_data_index=0;
	int c;
	while ((c = fgetc(fp)) != EOF)
	{
		file_data[file_data_index++] = c;
	}
	if (feof(fp))
	{
		num_of_bytes_in_file = file_data_index;
	}
	return file_data;
}


void sendPostToServer(SOCKET s)//socket s)
{

	WSADATA wsa;
	//SOCKET s;
	struct sockaddr_in server;
	char *message, server_reply[4];
	int recv_size;


	//Post details from user
	char post_description[204];
	char filename[40];

	struct PostDataClient *post_data = (struct PostDataClient*)malloc(sizeof(struct PostDataClient));

	printf("\nEnter the post description:");
	fflush(stdin);
	gets(post_description);

	printf("Enter the file name to be sent:");
	fflush(stdin);
	gets(filename);



	char *file_data;// [511 * 1024 + 40];
	file_data = getFileData(filename);



	//server.sin_addr.s_addr = inet_addr("192.168.137.1");
	server.sin_addr.s_addr = inet_addr("192.168.43.80");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{

		puts("connect error");
		return; //1;
	}

	int buffer[2];

	buffer[0] = 5;

	buffer[1] = num_of_bytes_in_file+strlen(filename)+strlen(post_description);


	
	
	send(s,(char*) buffer,2* sizeof(int), 0);

	//Receive a reply from the server
		
	if ((recv_size = recv(s, server_reply, 2000, 0)) > 0)
	{
		server_reply[recv_size] = '\0';
	}

	//puts("Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	
	if (strcmp(server_reply, "ok") == 0)
	{

		strcpy(post_data->post_description, post_description);
		strcpy(post_data->username, current_user);
		strcpy(post_data->post_content, file_data);
		strcat(post_data->post_content, filename);

		send(s, (char*)post_data, sizeof(struct PostDataClient), 0);
	
	}
	else if (strcmp(server_reply, "error") == 0)
	{
		printf("\nCannot make the post!!!\n");
		closesocket(s);
		WSACleanup();
		return;
	}

	closesocket(s);
	WSACleanup();
}


void main()
{
}