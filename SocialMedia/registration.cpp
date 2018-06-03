#define _CRT_SECURE_NO_WARNINGS
#define USERS_LIMIT 256
#define USER_STRUCT_SIZE 64
#define USER_START 1024
#define UNAME_SIZE 30
#define PWORD_SIZE 30
#define db_file_name "database.bin"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <string.h>

FILE* db_fptr;


typedef struct meta_data
{
	char unused[1020];
	int no_users;
}meta_data;

typedef struct user
{
	char uname[30];
	char pword[30];
	int key;
}user;

void init_database()
{

	meta_data* _meta_data = (meta_data*)malloc(1 * sizeof(meta_data));
	_meta_data->no_users = 0;

	fseek(db_fptr, 0, SEEK_SET);
	fwrite((void*)_meta_data, sizeof(meta_data), 1, db_fptr);
	fseek(db_fptr, 1024 * 1024 * 100-1, SEEK_SET);
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

int update_uname(char* uname, char* new_uname)
{
	int key = get_user_key(uname);
	if (key == -1)
	{
		return -1;
	}

	user* _user = (user*)calloc(1, sizeof(user));
	long offset = (key * USER_STRUCT_SIZE) + USER_START;
	fseek(db_fptr, offset, SEEK_SET);
	fread((void*)_user, sizeof(user), 1, db_fptr);

	strcpy(_user->uname, new_uname);
	offset = (key * USER_STRUCT_SIZE) + USER_START;
	fseek(db_fptr, offset, SEEK_SET);
	fwrite((void*)_user, sizeof(user), 1, db_fptr);

	return 1;
}

int update_pword(char* uname, char* pword)
{
	int key = get_user_key(uname);
	if (key == -1)
	{
		return -1;
	}

	user* _user = (user*)calloc(1, sizeof(user));
	long offset = (key * USER_STRUCT_SIZE) + USER_START;
	fseek(db_fptr, offset, SEEK_SET);
	fread((void*)_user, sizeof(user), 1, db_fptr);

	strcpy(_user->pword, pword);
	offset = (key * USER_STRUCT_SIZE) + USER_START;
	fseek(db_fptr, offset, SEEK_SET);
	fwrite((void*)_user, sizeof(user), 1, db_fptr);

	return 1;
}


void print_user_data()
{
	long offset = 0;
	meta_data* _meta_data = (meta_data*)calloc(1, sizeof(meta_data));
	user* _user = NULL;
	fseek(db_fptr, 0, SEEK_SET);
	fread((void*)_meta_data, sizeof(meta_data), 1, db_fptr);

	for (size_t i = 0; i < unsigned(_meta_data->no_users); i++)
	{
		offset = i * USER_STRUCT_SIZE + USER_START;
		_user = (user*)calloc(1, sizeof(user));
		fseek(db_fptr, offset, SEEK_SET);
		fread((void*)_user, sizeof(user), 1, db_fptr);
		printf("%s-%s-%d\n", _user->uname, _user->pword, _user->key);
	}
}

/*dummy users creater*/
void utility_func01()
{
	char* string;
	char* buffer;
	for (size_t i = 0; i < unsigned(257); i++)
	{
		string = (char*)calloc(100, sizeof(char));
		buffer = (char*)calloc(100, sizeof(char));

		strcpy(string, "hari");
		_itoa(i, buffer, 10);

		strcat(string, buffer);

		create_user(string, "irah3");
	}
	return;
}



int main()
{
	db_fptr = fopen(db_file_name, "rb+");
	if (db_fptr == NULL)
	{
		db_fptr = fopen(db_file_name, "wb");
		fclose(db_fptr);
		db_fptr = fopen(db_file_name, "rb+");
	}

	init_database();

	utility_func01();
	print_user_data();
	update_uname("hari254", "harihari");
	update_pword("harihari", "irahirah");
	print_user_data();
	char* x = get_uname(254);
	char* y = get_pword("harihari");
	printf("%s\n", x);
	printf("%s\n", y);

	fclose(db_fptr);

	_getch();
	return 0;
}