//
//  functions.c
//
//  Tristan Gonzalez, phone, a database program for keeping names
//  and phone numbers
//  Copyright (c) 2013 Tristan Gonzalez. All rights reserved.
//  rgonzale@darkterminal.net
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>
#include <openssl/blowfish.h>
#include "prototypes.h"

#define MAX_DATA 32
#define MAX_ROWS 10
#define KEY_SIZE 16

void die(const char *message)
{
	if (errno)
		perror(message);
	else
		fprintf(stderr, "%s\n", message);

	Usage();

	exit(1);
}

void Usage()
{
	fprintf(stderr, "Usage: main <database_file> -<action> <args>\n"
			"Available actions:\n"
			"-a add_record, <index> <name> <phone>, if index is omitted record "
			"is added to next availabe index\n"
			"-A arrange, gets rid of empty slots\n"
			"-c create_database, <size> if omitted size of 10 is used\n"
			"-C Ncurses interface\n"
			"-d delete_record, <index>, if index is omitted last record is " 
			"deleted\n"
			"-D delete_shift, <index>, delete and shift\n"
			"-e export\n"
			"-f find, <name>\n"
			"-h help, display usage\n"
			"-i import, create database first then it will import from index "
			"0 via stdin\n"
			"-I insert, <index>, insert record and shift\n"
			"-l list\n"
			"-r resize <newsize>\n"
			"-s sort, good to run Arrange first\n"
			"-S shell\n");
}

void Shell_Usage()
{
	fprintf(stderr, "Usage: <action> <args>\n"
			"Available actions:\n"
			"a add_record, <index> <name> <phone>, if index is omitted record "
			"is added to next available index\n"
			"A arrange, gets rid of empty slots\n"
			"c create_database, <size> if omitted size of 10 is used\n"
			"d delete_record, <index>, if index is omitted last record is " 
			"deleted\n"
			"D delete_shift, <index>, delete and shift\n"
			"e export\n"
			"f find, <name>\n"
			"h help, display usage\n"
			"i import, create database first then it will import from index"
			"0 via stdin\n"
			"I insert, <index>, insert record and shift\n"
			"l list\n"
			"q quit, exit shell\n"
			"r resize <newsize>\n"
			"s sort\n");
}

Connection *DatabaseLoad(const char *file, char *action)
{
	int signature = 0;
	size_t members_read = 0;

	Connection *conn = calloc(1, sizeof(Connection));
	if (conn == NULL)
		die("ERROR 86: could not calloc conn");

	struct Core *core_ptr = calloc(1, sizeof(struct Core));
	if (core_ptr == NULL)
		die("ERROR 90: could not calloc core_ptr");

	conn->core = core_ptr;

	struct Config *cnf_ptr = calloc(1, sizeof(struct Config));
	if (cnf_ptr == NULL)
		die("ERROR 96: could not calloc cnf_ptr");

	conn->core->cnf = cnf_ptr;

	struct Database *db_ptr = calloc(1, sizeof(struct Database));
	if (db_ptr == NULL)
		die("ERROR 102: could not calloc db");

	conn->core->db = db_ptr;

	if (strchr("acdefhilnrsACDIS", *action) == NULL) {
		die("ERROR 107: strchr could not find available action");

	} else if (*action == 'c') {
		conn->fp = fopen(file, "w");

		if (conn->fp == NULL)
			die("ERROR 113: could not open file");

	} else {
		conn->fp = fopen(file, "r+");

		if (conn->fp == NULL)
			die("ERROR 119: could not open file");

		// check signature
		members_read = fread(&signature, sizeof(int), 1, conn->fp);
		if (signature != 53281)
			die("ERROR 127: db file not weno");

		members_read = fread(conn->core->cnf, 
				sizeof(struct Config), 1, conn->fp);
		/* fprintf(stderr, "%d Config members read\n", members_read);  */

		conn->core->db->rows = calloc(conn->core->cnf->size, sizeof(struct Information));
		if (conn->core->db->rows == NULL)
			die("ERROR 127: could not allocate space for rows");

		members_read = fread(conn->core->db->rows, 
				sizeof(struct Information), conn->core->cnf->size, conn->fp);
		/* fprintf(stderr, "%d Database members read\n", members_read);  */
	}

	return conn;
}

void DatabaseCreate(Connection *conn, int *size)
{
	int i;

	if (*size > 0) {
		struct Information *info = calloc(*size, sizeof(struct Information));
		if (info == NULL)
			die("ERROR 144: could not allocate space for info");

		for (i = 0; i < *size; i++) {
			info[i].index = i;
		}
		conn->core->db->rows = info;
		conn->core->cnf->size = *size;
		conn->core->cnf->free_index = 0;
		conn->core->cnf->delete_index = 0;

	} else {
		struct Information *info = calloc(MAX_ROWS, sizeof(struct Information));
		if (info == NULL)
			die("ERROR 157: could not allocate space for info");

		for (i = 0; i < MAX_ROWS; i++) {
			info[i].index = i;
		}
		conn->core->db->rows = info;
		conn->core->cnf->size = MAX_ROWS;
		conn->core->cnf->free_index = 0;
		conn->core->cnf->delete_index = 0;
	}
}

void AddRecord(Connection *conn, int *index, 
		const char *name, const char *phone)
{
	int db_size = (conn->core->cnf->size) * 2;
	struct Information *rows = conn->core->db->rows;
	int *size = &(conn->core->cnf->size);
	int *free_index = &(conn->core->cnf->free_index);


	if (index == NULL) {
		if ((name == NULL) || (phone == NULL)) {
			fprintf(stderr, "ERROR 180: name or phone is NULL\n");
			exit(1);
		}
		strncpy(rows[*free_index].name, name, MAX_DATA); 
		strncpy(rows[*free_index].phone, phone, MAX_DATA); 

		RecalculateIndexes(conn);

	} else {
		if (*index > *size || *index < 0)
			die("ERROR 190: index out of bounds");
		if ((name == NULL) || (phone == NULL)) {
			fprintf(stderr, "ERROR 192: name or phone is NULL\n");
			exit(1);
		}
		strncpy(rows[*index].name, name, MAX_DATA);
		strncpy(rows[*index].phone, phone, MAX_DATA);

		RecalculateIndexes(conn);

	}
	while (*free_index >= (*size - 1)) {
		DatabaseResize(conn, &db_size);
		db_size *= 2;
	}

	RecalculateIndexes(conn);
}

void DeleteRecord(Connection *conn, int *index)
{
	int i;
	int db_size = (conn->core->cnf->size) / 2;
	struct Information *rows = conn->core->db->rows;
	int *size = &(conn->core->cnf->size);
	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);

	if (index == NULL) {

		if (*free_index == 0 && *delete_index == 0) {
			die("ERROR 219: database empty already");
		}

		for (i = 0; i < MAX_DATA; i++) {
			rows[*delete_index].name[i] = 0;
			rows[*delete_index].phone[i] = 0;
		}

	} else {
		if (*index > *size || *index < 0) 
			die("ERROR 231: index out of bounds");
		if (*free_index == 0 && *delete_index == 0) 
			die("ERROR 233: database empty already");
		for (i = 0; i < MAX_DATA; i++) {
			rows[*index].name[i] = 0;
			rows[*index].phone[i] = 0;
		}
	}

	RecalculateIndexes(conn);

	while (*delete_index < (db_size - 1)) {
		DatabaseResize(conn, &db_size);
		db_size /= 2;
	}

	RecalculateIndexes(conn);
}

void AddInsert(Connection *conn, int *index,
		const char *name, const char *phone)
{
	int i;
	struct Information *rows = conn->core->db->rows;
	int *delete_index = &(conn->core->cnf->delete_index);

	if (rows[*index].name == NULL)
		AddRecord(conn, index, name, phone);
	else  {
		for (i = (*delete_index); i >= (*index); i--) {
			memmove(&rows[i+1], &rows[i], sizeof(struct Information));
			rows[i+1].index++;
		}
		AddRecord(conn, index, name, phone);
	}
}

void DeleteInsert(Connection *conn, int *index)
{
	int i;
	struct Information *rows = conn->core->db->rows;
	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);

	if (rows[*index].name[0] == '\0')
		DeleteRecord(conn, index);
	else {
		for (i = *index; i < (*delete_index) + 1; i++) {
			memmove(&rows[i], &rows[i + 1], sizeof(struct Information));
			rows[i].index--;
		}
		(*delete_index)--;

		*free_index = 0;
		while (rows[*free_index].name[0] != 0)
			(*free_index)++;
	}
}

void DatabaseWrite(Connection *conn, const char *file)
{
	int signature = 53281; // 0xD021
	size_t members_written = 0;
	freopen(file, "w", conn->fp);

	members_written = fwrite(&signature, sizeof(int), 1, conn->fp);

	members_written = fwrite(conn->core->cnf, 
			sizeof(struct Config), 1, conn->fp);
	/* fprintf(stderr, "%d Config items written\n", members_written); */

	members_written = fwrite(conn->core->db->rows, 
			sizeof(struct Information), conn->core->cnf->size, conn->fp);
	/* fprintf(stderr, "%d Database items written\n", members_written); */
}

void DatabaseClose(Connection *conn)
{
	if (conn->fp) {
		fclose(conn->fp);
	} else {
		die("ERROR 308: file descriptor not found");
	}

	if (conn->core->db->rows) {
		free(conn->core->db->rows);
	} else {
		die("ERROR 314: unable to free conn->core->db->rows");
	}

	if (conn->core->db) {
		free(conn->core->db);
	} else {
		die("ERROR 320: unable to free conn->core->db");
	}

	if (conn->core->cnf) {
		free(conn->core->cnf);
	} else {
		die("ERROR 326: unable to free conn->core->cnf");
	}

	if (conn->core) {
		free(conn->core);
	} else {
		die("ERROR 332: unable to free conn->core");
	}

	if (conn) {
		free(conn);
	} else {
		die("ERROR 338: unable to free conn");
	}
}

void DatabaseList(Connection *conn, WINDOW *win)
{
	int i;
	/*fprintf(stderr, "delete_index %d\nfree_index %d\n",
	  conn->core->cnf->delete_index, 
	  conn->core->cnf->free_index);*/

	if (win) {
		for (i = 0; i < conn->core->cnf->size; i++) {
			wprintw(win, "%-2d %-10s %-10s\n", conn->core->db->rows[i].index, 
					conn->core->db->rows[i].name, 
					conn->core->db->rows[i].phone);
		}
	} else {
		for (i = 0; i < conn->core->cnf->size; i++) {
			printf("%-2d %-10s %-10s\n", conn->core->db->rows[i].index, 
					conn->core->db->rows[i].name, 
					conn->core->db->rows[i].phone);
		}
	}
}

void DatabaseResize(Connection *conn, int *newsize)
{
	int i;
	struct Information *rows = conn->core->db->rows;
	int *size = &(conn->core->cnf->size);
	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);
	/* printf("size = %i\nfree_index = %i\ndelete_index = %i\n",  */
	/* *size, *free_index, *delete_index); */

	if (*newsize > *size) {
		struct Information *info = calloc(*size, sizeof(struct Information));
		if (info == NULL)
			die("ERROR 377: could not allocate space for info");

		for (i = 0; i < *size; i++) {
			info[i] = rows[i];
		}

		free(rows);

		struct Information *newinfo = calloc(*newsize, sizeof(struct Information));
		if (newinfo == NULL)
			die("ERROR 387: could not calloc newinfo");

		rows = newinfo;

		for (i = 0; i < *newsize; i++) {
			rows[i].index = i;
		}
		for (i = 0; i < conn->core->cnf->size; i++) {
			rows[i] = info[i];
		}

		free(info);
		conn->core->db->rows = rows;

	} else if (*size == *newsize) {
		die("ERROR 402: resize, old size is the same as the new size");
	} else if (*newsize <= *free_index || 
			*newsize <= (*delete_index + 1)) {
		die("ERROR 405: resize: cannot resize below valid records");
	}
	else  {
		struct Information *newinfo = calloc(*newsize, sizeof(struct Information)); 
		if (newinfo == NULL)
			die("ERROR 410: could not calloc newinfo");

		for (i = 0; i < *newsize; i++) {
			newinfo[i] = rows[i];
		}

		free(rows);
		rows = newinfo;
		conn->core->db->rows = rows;
	}
	*size = *newsize;
}

void DatabaseExport(Connection *conn)
{
	int i;
	for (i = 0; i <= conn->core->cnf->delete_index; i++)
		printf("%s %s\n", 
				conn->core->db->rows[i].name, 
				conn->core->db->rows[i].phone);
}

void DatabaseImport(Connection *conn)
{
	int i = 0;
	int db_size;
	char name[MAX_DATA];
	char phone[MAX_DATA];

	conn->core->cnf->free_index = 0;
	conn->core->cnf->delete_index = 0;

	while(scanf("%s %s", name, phone) != EOF) {
		if (conn->core->cnf->size - i == 1) {
			db_size = i + 10;
			DatabaseResize(conn, &db_size);
		}
		AddRecord(conn, &i, name, phone);
		i++;
	}
}

void DatabaseFind(Connection *conn, char *name, WINDOW *win)
{
	int i;
	if (win) {
		for (i = 0; i < conn->core->cnf->size; i++) {
			if (strncmp(conn->core->db->rows[i].name, name, MAX_DATA) == 0)
				wprintw(win, "%d %s %s\n", conn->core->db->rows[i].index,
						conn->core->db->rows[i].name,
						conn->core->db->rows[i].phone);
		}
	} else {
		for (i = 0; i < conn->core->cnf->size; i++) {
			if (strncmp(conn->core->db->rows[i].name, name, MAX_DATA) == 0)
				printf("%d %s %s\n", conn->core->db->rows[i].index,
						conn->core->db->rows[i].name,
						conn->core->db->rows[i].phone);
		}
	}

}

void DatabaseSort(Connection *conn)
{
	int i, j;
	char tmp1[MAX_DATA];
	char tmp2[MAX_DATA];

	struct Information *rows = conn->core->db->rows;
	int *delete_index = &(conn->core->cnf->delete_index);

	for (i = 0; i <= *delete_index; i++) {
		for (j = i + 1; j <= *delete_index; j++) {
			if (rows[j].name[0] == 0)
				break;
			if ((strncmp(rows[i].name, rows[j].name, MAX_DATA - 1)) > 0) {
				strncpy(tmp1, rows[j].name, MAX_DATA - 1);
				strncpy(tmp2, rows[j].phone, MAX_DATA - 1);
				strncpy(rows[j].name, rows[i].name, MAX_DATA - 1);
				strncpy(rows[j].phone, rows[i].phone, MAX_DATA - 1);
				strncpy(rows[i].name, tmp1, MAX_DATA - 1);
				strncpy(rows[i].phone, tmp2, MAX_DATA - 1);
			}
		}
	}
}

void DatabaseArrange(Connection *conn)
{
	int i = 0;
	int j = 0;
	int x = 0;
	int db_size = (conn->core->cnf->size) / 2;
	struct Information *rows = conn->core->db->rows;
	int *delete_index = &(conn->core->cnf->delete_index);

	while (j < (*delete_index)) {
		while (rows[j].name[0] == '\0') {
			do {
				x++;
			} while (rows[j + x].name[0] == '\0');

			memmove(&rows[j], &rows[j + x], sizeof(struct Information));
			if ((*delete_index) == j + x)
				*delete_index = j;
			for (i = 0; i < MAX_DATA; i++) {
				rows[j + x].name[i] = 0;
				rows[j + x].phone[i] = 0;
			}
			rows[j].index = j;
		}
		j++;
		x = 0;
	}

	RecalculateIndexes(conn);

	while (*delete_index < (db_size - 1)) {
		DatabaseResize(conn, &db_size);
		db_size /= 2;
	}

	RecalculateIndexes(conn);
}

void ParseArguments(Connection *conn, const char *file, char *args)
{
	int index;
	char *arg1;
	char *arg2;
	char *arg3;
	char *arg4;

	arg1 = strtok(args, " ");
	arg2 = strtok(NULL, " ");
	arg3 = strtok(NULL, " ");
	arg4 = strtok(NULL, " ");

	if (strncmp(arg1, "list", 1) == 0) {
		DatabaseList(conn, NULL);
	}
	if (strncmp(arg1, "export", 1) == 0) {
		DatabaseExport(conn);
	}
	if (strncmp(arg1, "import", 1) == 0) {
		DatabaseImport(conn);
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "Insert", 1) == 0) {
		index = atoi(arg2);
		AddInsert(conn, &index, arg3, arg4);
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "add", 1) == 0) {
		if (arg4 == NULL) {
			AddRecord(conn, NULL, arg2, arg3);
		} else {
			index = atoi(arg2);
			AddRecord(conn, &index, arg3, arg4);
		}
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "delete", 1) == 0) {
		if (arg2 == NULL) {
			DeleteRecord(conn, NULL);
		} else {
			index = atoi(arg2);
			DeleteRecord(conn, &index);
		}
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "DeleteShift", 1) == 0) {
		if (arg2 == NULL) {
			DeleteInsert(conn, NULL);
		} else {
			index = atoi(arg2);
			DeleteInsert(conn, &index);
		}
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "resize", 1) == 0) {
		index = atoi(arg2);
		DatabaseResize(conn, &index);
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "Arrange", 1) == 0) {
		DatabaseArrange(conn);
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "sort", 1) == 0) {
		DatabaseSort(conn);
		DatabaseWrite(conn, file);
	}
	if (strncmp(arg1, "find", 1) == 0) {
		DatabaseFind(conn, arg2, NULL);
	}
	if (strncmp(arg1, "help", 1) == 0) {
		Shell_Usage();
	}
}

void DatabaseShell(Connection *conn, const char *file)
{
	void mysighand(int signum) {
		if (signum == 2) {
			fprintf(stderr, "Catching SIGINT\nClosing DB\n");
			DatabaseClose(conn);
			exit(1);
		}
	}

	signal(SIGINT, mysighand);

	char input[MAX_DATA];

	do {
		printf("shell> ");
		if (fgets(input, MAX_DATA, stdin) == NULL) {
			fprintf(stderr, "Exiting program\n"
					"Closing DB\n");
			DatabaseClose(conn);
			exit(1);
		}
		if (input[0] != '\n') {
			input[(strlen(input)) - 1] = '\0';
			ParseArguments(conn, file, input);
		}
	} while (strncmp(input, "quit", 1)); 
}

void RecalculateIndexes(Connection *conn)
{
	struct Information *rows = conn->core->db->rows;
	int *size = &(conn->core->cnf->size);
	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);

	*free_index = 0;
	while (rows[*free_index].phone[0] != 0)
		(*free_index)++;
	*delete_index = (*size) - 1;
	while (rows[*delete_index].name[0] == 0)
		(*delete_index)--;
}

void BlowFish(Connection *conn, char *action)
{
	struct Information *rows = conn->core->db->rows;
	int *size = &(conn->core->cnf->size);

	unsigned char *in;
	unsigned char *out = calloc(MAX_DATA, sizeof(char));
	BF_KEY *key = calloc(1, sizeof(BF_KEY));
	FILE *fkey;
	char *keybuf[16];

	fkey = fopen("~/.weno/.key", "r");
	fread(keybuf, 16, 1, fkey);

	BF_set_key(key, KEY_SIZE, (const unsigned char*)keybuf);

	if (action == 'e') {
		BF_ecb_encrypt(in, out, key, BF_ENCRYPT);
		BF_ecb_encrypt(in + 8, out + 8, key, BF_ENCRYPT);
	}

	if (action == 'd') {
		BF_ecb_encrypt(out, out2, key, BF_DECRYPT);
		BF_ecb_encrypt(out + 8, out2 + 8, key, BF_DECRYPT);
	}
}
