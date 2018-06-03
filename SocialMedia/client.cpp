//#define _CRT_SECURE_NO_WARNINGS
//
//#include<stdio.h>
//#include<stdlib.h>
//#include<string.h>
//
//#define MAX_POST_SIZE 511*1024+760
//#define POST_START_POINTER 148480
//
//
//FILE *db_ptr;
//char *current_user;
//int num_of_bytes_in_file;
//
//
//
//
//
//struct PostDataClient
//{
//	char username[20];
//	char post_description[204];
//	char *post_content;
//};
//
//
//struct PostDataServer
//{
//	char username[20];
//	char post_description[204];
//	char *post_content;
//};
//
//
//int getUserId(char username[])
//{
//	return 1;
//}
//
//
//void getPostFromClient1(int *data)
//{
//	if (data[0] == 5)//5 is Code for post
//	{
//		if (data[1] < MAX_POST_SIZE)
//		{
//			msg = "ok";
//			return msg;
//		}
//		else
//		{
//			msg = "error";
//			return msg;
//		}
//
//	}
//}
//
//
//void getPostFromClient2(char *post_data_from_client)//SOCKET s)
//{
//	int recv_size;
//	char client_msg[5];
//	char post_data_from_client[MAX_POST_SIZE];
//	char msg_to_be_stored[MAX_POST_SIZE];
//	int post_meta_data_offset;
//
//	
//		
//		struct PostDataServer *post_from_client = (struct PostDataServer*)malloc(sizeof(struct PostDataServer));
//
//		post_from_client = (struct PostDataServer*)post_data_from_client;
//
//		fseek(db_ptr, 4, SEEK_SET);//To read the post id
//
//		fread(&post_meta_data_offset, sizeof(int), 1, db_ptr);//Reads the post number into the offset
//
//		strcpy(msg_to_be_stored, (char*)post_meta_data_offset);
//
//		strcat(msg_to_be_stored, (char*)getUserId(post_from_client->username));
//
//		strcat(msg_to_be_stored, post_data_from_client);
//
//		fseek(db_ptr, (post_meta_data_offset * 512 * 1024) + POST_START_POINTER, SEEK_SET);
//
//		fwrite(msg_to_be_stored, sizeof(post_data_from_client), 1, db_ptr);
//	
//
//	//server_recv = (int*)client_msg;
//}
//
//
//
//char* getFileData(char *filename)
//{
//	FILE *fp = fopen(filename, "rb");
//	char file_data[511 * 1024];
//	int file_data_index = 0;
//	int c;
//	while ((c = fgetc(fp)) != EOF)
//	{
//		file_data[file_data_index++] = c;
//	}
//	if (feof(fp))
//	{
//		num_of_bytes_in_file = file_data_index;
//	}
//	return file_data;
//}
//
//
//void sendPostToServer()//SOCKET s)//socket s)
//{
//
//	//WSADATA wsa;
//	//SOCKET s;
//	//struct sockaddr_in server;
//	char *message, server_reply[4];
//	int recv_size;
//
//
//	//Post details from user
//	char post_description[204];
//	char filename[40];
//
//	struct PostDataClient post_data;
//
//	printf("\nEnter the post description:");
//	fflush(stdin);
//	gets(post_description);
//
//	printf("Enter the file name to be sent:");
//	fflush(stdin);
//	gets(filename);
//
//
//
//	char *file_data;// [511 * 1024 + 40];
//	file_data = getFileData(filename);
//
//	int buffer[2];
//
//	buffer[0] = 5;
//
//	buffer[1] = num_of_bytes_in_file + strlen(filename) + strlen(post_description);
//
//
//
//	server_reply = getPostFromClient1(buffer);
//
//	//Add a NULL terminating character to make it a proper string before printing
//	server_reply[3] = '\0';
//
//	if (strcmp(server_reply, "ok") == 0)
//	{
//
//		strcpy(post_data.post_description, post_description);
//		strcpy(post_data.username, current_user);
//		strcpy(post_data.post_content, file_data);
//		strcat(post_data.post_content, filename);
//
//		getPostFromClient2((char*)&post_data);
//	}
//	else if (strcmp(server_reply, "er") == 0)
//	{
//		printf("\nCannot make the post!!!\n");
//		return;
//	}
//}
//
//
//void main()
//{
//	db_ptr = fopen("xyz.bin", "ab+");
//	sendPostToServer();
//}




























//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
//
//#include<stdio.h>
//#include<winsock2.h>
//
//#pragma comment(lib,"ws2_32.lib") //Winsock Library
//
//int main(int argc, char *argv[])
//{
//	WSADATA wsa;
//	SOCKET s;
//	struct sockaddr_in server;
//	char *message, server_reply[5000];
//	int recv_size;
//
//	printf("\nInitialising Winsock...");
//	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//	{
//		printf("Failed. Error Code : %d", WSAGetLastError());
//		return 1;
//	}
//
//	printf("Initialised.\n");
//
//	//Create a socket
//	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
//	{
//		printf("Could not create socket : %d", WSAGetLastError());
//	}
//
//	printf("Socket created.\n");
//
//
//	//server.sin_addr.s_addr = inet_addr("192.168.137.1");
//	server.sin_addr.s_addr = inet_addr("192.168.43.80");
//	server.sin_family = AF_INET;
//	server.sin_port = htons(8888);
//
//	//Connect to remote server
//	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
//	{
//	
//		puts("connect error");
//		return 1;
//	}
//
//	char buffer[100];
//	puts("Connected");
//	while (1){
//		printf("\nEnter message: ");
//		scanf("%s", buffer);
//		send(s, buffer, strlen(buffer), 0);
//		//Receive a reply from the server
//		if ((recv_size = recv(s, server_reply, 2000, 0)) > 0)
//		{
//			server_reply[recv_size] = '\0';
//			printf("%s\n", server_reply);
//		}
//	}
//
//	puts("Reply received\n");
//
//	//Add a NULL terminating character to make it a proper string before printing
//	server_reply[recv_size] = '\0';
//	puts(server_reply);
//
//
//	closesocket(s);
//	WSACleanup();
//
//	return 0;
//}