//
//  prototypes.h
//
//  Tristan Gonzalez, database program for keeping names and phone numbers
//  Copyright (c) 2013 Tristan Gonzalez. All rights reserved.
//	rgonzale@darkterminal.net
//

#include <ncurses.h>

#define MAX_DATA 32
#define MAX_ROWS 10

void die(const char *message);

void Usage();

void Shell_Usage();

struct Connection {
   FILE *fp;
   struct Core *core;
};

typedef struct Connection Connection;

struct Core {
	struct Config *cnf;
	struct Database *db;
};

struct Config {
	int size;
	int free_index;
	int delete_index;
};

struct Database {
   struct Information *rows;
};

struct Information {
   int index;
   char name[MAX_DATA];
   char phone[MAX_DATA];
};

void DatabaseCreate(Connection *conn, int *size);

struct Connection *DatabaseLoad(const char *file, char *action);

void DatabaseClose(Connection *conn);

void AddRecord(Connection *conn, int *index, const char *name, const char *phone);

void DeleteRecord(Connection *conn, int *index);

void AddInsert(Connection *conn, int *index, const char *name, const char *phone);

void DeleteInsert(Connection *conn, int *index);

void DatabaseWrite(Connection *conn, const char *file);

void DatabaseClose(Connection *conn);

void DatabaseList(Connection *conn, WINDOW *win);

void DatabaseResize(Connection *conn, int *newsize);

void DatabaseImport(Connection *conn);

void DatabaseFind(Connection *conn, char *name,	WINDOW *win);

void DatabaseExport(Connection *conn);

void DatabaseSort(Connection *conn);

void DatabaseShell(Connection *conn, const char *file);

void ParseArguments(Connection *conn, const char *file, char *args);

void RecalculateIndexes(Connection *conn);

void DatabaseArrange(Connection *conn);

// Ncurses prototypes
void DatabaseNcurses(Connection *conn, const char *file);

void NcursesCenter(WINDOW *win, int row, const char *title);

void NcursesControl(Connection *conn, const char *file);

void NcursesUsage();

void NcursesConsole(Connection *conn, const char *file);

void NcursesExamine(Connection *conn, const char *file);

void ExamineSelection(int *selection, Connection *conn);

void NcursesResize(Connection *conn, const char *file);

void DisplayError(const char *message);

void RefreshdbList(Connection *conn);

void NcursesOtherdb(Connection *conn, const char *file);

void OtherdbSelection(int *selection);

void OtherdbListing();

void ReloadListing();

void DisplayMode(const char *message);

void ClearLine(WINDOW *win);

void Secrecy(Connection *conn, char *action);
