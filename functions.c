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
#include "prototypes.h"

#define MAX_DATA 32
#define MAX_ROWS 10

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
			"-s sort\n"
			"-S shell\n");
}

void Shell_Usage()
{
	fprintf(stderr, "Usage: <action> <args>\n"
			"Available actions:\n"
			"a add_record, <index> <name> <phone>, if index is omitted record "
			"is added to next available index\n"
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
	size_t members_read = 0;

	Connection *conn = calloc(1, sizeof(link));
	if (conn == NULL)
		die("ERROR 82: could not calloc conn");

	struct Core *core_ptr = calloc(1, sizeof(struct Core));
	if (core_ptr == NULL)
		die("ERROR 88: could not calloc core_ptr");

	conn->core = core_ptr;

	struct Config *cnf_ptr = calloc(1, sizeof(struct Config));
	if (cnf_ptr == NULL)
		die("ERROR 96: could not calloc cnf_ptr");

	conn->core->cnf = cnf_ptr;

	struct Database *db_ptr = calloc(1, sizeof(struct Database));
	if (db_ptr == NULL)
		die("ERROR 104: could not calloc db");

	conn->core->db = db_ptr;

	if (strchr("acdefhilrsACDIS", *action) == NULL) {
		die("ERROR 1110: strchr could not find available action");

	} else if (*action == 'c') {
		conn->fp = fopen(file, "w");

		if (conn->fp == NULL)
			die("ERROR 116: could not open file");

	} else {
		conn->fp = fopen(file, "r+");

		if (conn->fp == NULL)
			die("ERROR 122: could not open file");

		members_read = fread(conn->core->cnf, 
				sizeof(struct Config), 1, conn->fp);
		/* fprintf(stderr, "%d Config members read\n", members_read);  */

		conn->core->db->rows = calloc(conn->core->cnf->size, sizeof(struct Information));
		if (conn->core->db->rows == NULL)
			die("ERROR 134: could not allocate space for rows");

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
			die("ERROR 151: could not allocate space for info");

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
			die("ERROR 164: could not allocate space for info");

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
			fprintf(stderr, "ERROR 186: name or phone is NULL\n");
			exit(1);
		}
		strncpy(rows[*free_index].name, name, MAX_DATA); 
		strncpy(rows[*free_index].phone, phone, MAX_DATA); 

		RecalculateIndexes(conn);

	} else {
		if (*index > *size || *index < 0)
			die("ERROR 202: index out of bounds");
		if ((name == NULL) || (phone == NULL)) {
			fprintf(stderr, "ERROR 211: name or phone is NULL\n");
			exit(1);
		}
		strncpy(rows[*index].name, name, MAX_DATA);
		strncpy(rows[*index].phone, phone, MAX_DATA);

		RecalculateIndexes(conn);

	}
	if (*free_index >= *size)
		DatabaseResize(conn, &db_size);

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
			die("ERROR 241: database empty already");
		}

		for (i = 0; i < MAX_DATA; i++) {
			rows[*delete_index].name[i] = 0;
			rows[*delete_index].phone[i] = 0;
		}

		RecalculateIndexes(conn);

	} else {
		if (*index > *size || *index < 0) 
			die("ERROR 257: index out of bounds");
		if (*free_index == 0 && *delete_index == 0) 
			die("ERROR 259: database empty already");
		for (i = 0; i < MAX_DATA; i++) {
			rows[*index].name[i] = 0;
			rows[*index].phone[i] = 0;
		}

		RecalculateIndexes(conn);

	}

	if (*delete_index < (db_size - 1))
		DatabaseResize(conn, &db_size);

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
	size_t members_written = 0;
	freopen(file, "w", conn->fp);

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
		die("ERROR 351: file descriptor not found");
	}

	if (conn->core->db->rows) {
		free(conn->core->db->rows);
	} else {
		die("ERROR 357: unable to free conn->core->db->rows");
	}

	if (conn->core->db) {
		free(conn->core->db);
	} else {
		die("ERROR 363: unable to free conn->core->db");
	}

	if (conn->core->cnf) {
		free(conn->core->cnf);
	} else {
		die("ERROR 369: unable to free conn->core->cnf");
	}

	if (conn->core) {
		free(conn->core);
	} else {
		die("ERROR 375: unable to free conn->core");
	}

	if (conn) {
		free(conn);
	} else {
		die("ERROR 381: unable to free conn");
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
			wprintw(win, "%d %s %s\n", conn->core->db->rows[i].index, 
					conn->core->db->rows[i].name, 
					conn->core->db->rows[i].phone);
		}
	} else {
		for (i = 0; i < conn->core->cnf->size; i++) {
			printf("%d %s %s\n", conn->core->db->rows[i].index, 
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
			die("ERROR 384: could not allocate space for info");

		for (i = 0; i < *size; i++) {
			info[i] = rows[i];
		}

		free(rows);

		struct Information *newinfo = calloc(*newsize, sizeof(struct Information));
		if (newinfo == NULL)
			die("ERROR 432: could not calloc newinfo");

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
		die("ERROR 448: resize, old size is the same as the new size");
	} else if (*newsize <= *free_index || 
			*newsize <= (*delete_index + 1)) {
		die("ERROR 451: resize: cannot resize below valid records");
	}
	else  {
		struct Information *newinfo = calloc(*newsize, sizeof(struct Information)); 
		if (newinfo == NULL)
			die("ERROR 457: could not calloc newinfo");

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

	if (*delete_index < (db_size - 1))
		DatabaseResize(conn, &db_size);

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

