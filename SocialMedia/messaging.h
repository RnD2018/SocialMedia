#define MESSAGE_META 12
#define RQ_TYPE_SEND_MSG 3
#define RQ_TYPE_GET_ALL_MSGS 4

/* Client side structure of the message to be sent*/
typedef struct _message{
	char sender[20];
	char receiver[20];
	char* message;
}_message;

/* Structure of the message to be sent from server, for get all messages from user*/
typedef struct _s_message{
	int size;
	char sender[20];
	char* message;
}_s_message;

//Client
void send_message(char* user_name,SOCKET s)
{
	printf("Enter the username to whom you want to send : ");
	char* receiver = (char*)malloc(20 * sizeof(char));
 
	scanf("%s", receiver);

	printf("Enter message : ");
	char* user_message = (char*)malloc(500 * sizeof(char));
	fflush(stdin);
	scanf("%[^\n]s", user_message);

	int* request_arr = (int*)malloc(2 * sizeof(int));
	request_arr[0] = RQ_TYPE_SEND_MSG;
	request_arr[1] = strlen(user_message) + 40;

	send(s, (char*)request_arr, 8, 0);

	//char* server_reply = (char*)malloc(4);
	int response_size;
	int* server_reply = (int*)malloc(2 * sizeof(int));
	if ((response_size = recv(s, (char*)server_reply, 8, 0)) > 0)
	{
		//if (strcmp(server_reply, "OK") == 0)
		if (server_reply[0]==1)
		{
			_message* u_message = (_message*)malloc(sizeof(_message));
			u_message->message = user_message;
			strcpy(u_message->sender, user_name);
			strcpy(u_message->receiver, receiver);

			send(s, (char*)u_message, 540, 0);
			
			printf("Message sent successfully!\n");
			free(u_message);
		}
	}
	free(receiver);
	free(user_message);
	free(server_reply);
	free(request_arr);
	
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


_s_message** extract_messages(char* server_reply,int* len){
	char* temp=server_reply;
	*len = 50;
	_s_message** messages = (_s_message**)malloc(50 * sizeof(_s_message*));
	int message_count = 0;
	while (1)
	{
		int size = *((int*)(temp + 4));
		temp += 4;
		if (size == -1)
			break;
		char* username = (char*)calloc(20 , sizeof(char));
		for (int i = 0; i < 20; i++)
		{
			if (((temp[i] >= 'a'&&temp[i] <= 'z') || (temp[i] >= 'A'&&temp[i] <= 'Z') || (temp[i] >= '0'&&temp[i] <= '9') || temp[i] == '_'))
				username[i] = temp[i];
			else{
				username[i] = '\0';
				break;
			}
		}
		temp += 20;
		char* message = (char*)calloc(size + 1, 1);
		for (int i = 0; i < size; i++)
			message[i] = temp[i];

		_s_message* reply = (_s_message*)malloc(sizeof(_s_message));
		
		strcpy(reply->message, username);
		reply->message = message;
		if (message_count >= *len)
			extend_size(messages, len);
		messages[message_count++] = reply;
	}
	*len = message_count;
	return messages;

}

_s_message** get_all_messages(char* user_name, SOCKET s,int* message_count){
	
	int* request_arr = (int*)malloc(2 * sizeof(int));
	request_arr[0] = RQ_TYPE_GET_ALL_MSGS;
	request_arr[1] = -1;
	send(s, (char*)request_arr, 8, 0);

	//_s_message** messages;

	//char* server_reply = (char*)malloc(4);
	int response_size;
	int* server_reply = (int*)malloc(2 * sizeof(int));
	int size;
	char* server_messages;
	if ((response_size = recv(s, (char*)server_reply, 8, 0)) > 0)
	{
		//if (strcmp(server_reply, "OK") == 0)
		if (server_reply[0] == 1)
		{
			size = server_reply[1];
			send(s, user_name, 30, 0);
		}
	}
	server_messages = (char*)malloc(size);
	if ((response_size = recv(s, server_messages, size, 0)) > 0)
	{
		return extract_messages(server_messages, message_count);
	}
	return NULL;

}

//server
FILE* db_fptr;
int get_user_key(char* user_name){
	return 0;
}

char* get_uname(int key){
	return NULL;
}

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
char** break_message(char* reply,int size)
{
	char** broken_message = (char**)malloc(3 * sizeof(char*));

	char* sender = (char*)calloc(20,1);
	for (int i = 0; i < 20; i++)
	{
		sender[i] = reply[i];
	}
	
	char* receiver = (char*)calloc(20,1);
	for (int i = 0; i < 20; i++)
	{
		receiver[i] = reply[20 + i];
	}
	char* message = (char*)calloc(size - 39, 1);
	for (int i = 0; i < size - 40; i++)
	{
		message[i] = reply[40 + i];
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

///* Structure of the message to be sent from server, for get all messages from user*/
//typedef struct _s_message{
//	char sender[20];
//	int size;
//	char* message;
//}_s_message;

void insert_message(char** broken_mesage,int size)
{
	char* sender = broken_mesage[0];
	char* receiver = broken_mesage[1];
	char* message = broken_mesage[2];

	int offset = get_messages_offset();
	
	int present_offset = offset + size - 40 + sizeof(message_info);
	fseek(db_fptr, -(present_offset-4), SEEK_END);
	int check_size = -1;
	fwrite(&check_size, 4, 1, db_fptr);
	fwrite(message, size - 40, 1, db_fptr);

	message_info* info = (message_info*)malloc(sizeof(message_info));
	info->sender_key = get_user_key(sender);
	info->receiver_key = get_user_key(receiver);
	info->size = size-40;
	fwrite(info, sizeof(message_info), 1, db_fptr);
	free(info);
	set_messages_offset(present_offset);

}

void receive_message(SOCKET s, int size)
{
	char* reply = (char*)malloc(size);
	int reply_size;

	if ((reply_size = recv(s, reply, size + 1, 0)) > 0)
	{
		char** broken_message = break_message(reply, size);
	}
}


_s_message** get_user_messages(char* user_name,int* len,int* total_size){
		
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

char* get_message_string(_s_message** messages,int message_count,int bytes_size)
{
	
	char* message = (char*)calloc(bytes_size+4, 1);
	int index = 0;
	for (int i = 0; i < message_count; i++)
	{
		int size = messages[i]->size;
		*((int*)message + index) = size;
		index += 4;
		for (int i = 0; i < 20; i++)
		{
			message[index + i] = messages[i]->sender[i];
		}
		index += 20;
		for (int i = 0; i < size; i++)
		{
			message[index + i] = messages[i]->message[i];
		}
		index += size;
	}
	*((int*)message + index) = -1;
	return message;
}

void response_all_messages(SOCKET s,char* user_name)
{
	int message_count;
	int size_bytes;
	_s_message** messages = get_user_messages(user_name, &message_count, &size_bytes);

	size_bytes += (20 + 4)*message_count;
	char* message = get_message_string(messages, message_count,size_bytes);

	send(s, message, size_bytes+4, 0);
	printf("Messages sent to user!\n");
}
