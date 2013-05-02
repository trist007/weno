//
//  ncurses-main.c
//
//  Tristan Gonzalez, database program for keeping names and phone numbers
//  Copyright (c) 2013 Tristan Gonzalez. All rights reserved.
//	rgonzale@darkterminal.net
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>
#include <dirent.h>
#include "prototypes.h"

#define MAX_DATA 32
#define PREFRESH prefresh(body, 0, 0, 6, 2, (maxy  / 2) + 8, maxx - 3)
#define PREFRESH_SCROLL prefresh(body, down, 0, 6, 2, (maxy  / 2) + 8, maxx - 3)
#define SUBWINDOWS newwin(3, (maxx/3) - 2, maxy / 3, maxx / 3)

char banner[] = "weno shell v0.1 by Tristan Gonzalez - Copyright 2013";
char help[] = {"[Master modes]\n"
	" c - console, type in commands\n"
		" e - examine, add/delete highlighted index\n"
		" n - encrypt/decrypt, <Coming Soon!>\n"
		" o - otherdb, other databases\n"
		" q = quit program\n"
		" r = resize ncurses\n\n"

		"[Commands]\n "

		"a - add_record, <index> <name> <phone>, if index is "
		"omitted record added to next available index\n "
		"A - arrange, gets rid of the empty slots\n "
		"c - create, new db, only in otherdb mode\n "
		"d - delete_record, <index>, if omitted last record "
		"deleted\n "
		"D - delete_insert, <index>, delete and shift\n "
		"e - execute, load db, only in otherdb mode\n "
		"I - insert, <index>, insert record and shift\n "
		"f - find, <name>\n "
		"p - phone, <index>, dials number, <Coming Soon!>\n "
		"q - quit, exit mode\n "
		"r - resize <newsize>\n "
		"s - sort, sorts alphabetically, best to arrange first"};

// global variables used by NcursesExamine and NcursesOtherdb
int gety, getx;

// global variables for DatabaseExamine
int down;

// global variables used throughout and initialized in NcursesResize
int maxy, maxx, halfy, halfx;

// global variables for usage window
int usage_y, usage_x, usage_maxy, usage_maxx;
char *help_ptr = help;

// global variables for DatabaseOther
int lscount;
char *ls_buf[256];
char lsbuf[128][32];

// global variables for windows initialized in NcursesResize
WINDOW *title, *border_body, *body, *border_console, *console,
	   *border_usage, *usage, *add, *resize, *find, *create;

void NcursesCenter(WINDOW *win, int row, const char *title)
{
	int len, indent, y, width;

	getmaxyx(win, y, width);

	len = strlen(title);
	indent = width - len;
	indent /= 2;

	mvwaddstr(win, row, indent, title);
	refresh();
}

void NcursesUsage()
{
	DisplayMode("Mode: Usage");
	touchwin(border_usage);
	wrefresh(border_usage);
	touchwin(usage);
	wrefresh(usage);
	getch();
	PREFRESH;
	refresh();
}

void NcursesConsole(Connection *conn, const char *file)
{
	char console_buf[MAX_DATA];
	char *arg1, *arg2, *arg3, *arg4; 

	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);
	int *size = &(conn->core->cnf->size);

	wmove(console, 1, 1);
	touchwin(console);
	wrefresh(console);
	napms(100);
	wbkgd(border_console, COLOR_PAIR(5));
	touchwin(border_console);
	wrefresh(border_console);
	touchwin(console);
	wrefresh(console);
	echo();

	do {
		DisplayMode("Mode: Console");
		wmove(body, 0, 0);
		werase(body);
		DatabaseList(conn, body);
		PREFRESH;

		wgetnstr(console, console_buf, MAX_DATA);
		arg1 = strtok(console_buf, " ");
		arg2 = strtok(NULL, " ");
		arg3 = strtok(NULL, " ");
		arg4 = strtok(NULL, " ");

		ClearLine(console);

		if (arg1 != NULL) {

			if (strncmp(arg1, "add", 1) == 0) {
				if (arg4 == NULL) {
					if ((arg2 == NULL) || (arg3 == NULL)) {
						DisplayError("name or phone is empty\n");
					} else
						AddRecord(conn, NULL, arg2, arg3);
				} else {
					int i = atoi(arg2);
					if (i > *size || i < 0) {
						DisplayError("index out of bounds\n");
					} else {
						if ((arg3 == NULL) || (arg4 == NULL)) {
							DisplayError("name or phone is empty\n");
						} else {
							AddRecord(conn, &i, arg3, arg4);
						}
					}
				}
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "Insert", 1) == 0) {
				if (arg4 == NULL) {
					AddInsert(conn, NULL, arg2, arg3);
				} else {
					int i = atoi(arg2);
					if (i > *size || i < 0) {
						DisplayError("index out of bounds\n");
					} else {
						AddInsert(conn, &i, arg3, arg4);
					}
				}
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "delete", 1) == 0) {
				if (*free_index == 0 && *delete_index == 0) {
					DisplayError("database is already empty\n");
				} else if (arg2 == NULL) {
					DeleteRecord(conn, NULL);
				} else {
					int i = atoi(arg2);
					DeleteRecord(conn, &i);
				}
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "Delete", 1) == 0) {
				if (*free_index == 0 && *delete_index == 0) {
					DisplayError("database is already empty\n");
				} else if (arg2 == NULL) {
					DeleteInsert(conn, NULL);
				} else {
					int i = atoi(arg2);
					DeleteInsert(conn, &i);
				}
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "resize", 1) == 0) {
				int i = atoi(arg2);
				if (i == *size) {
					DisplayError("database size is already that size\n");
				} else if (i <= *free_index || i <= (*delete_index) + 1) {
					DisplayError("cannot resize below valid records\n");
				} else {
					DatabaseResize(conn, &i);
					DatabaseWrite(conn, file);
				}
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
				wmove(body, 0, 0);
				werase(body);
				DatabaseFind(conn, arg2, body);
				PREFRESH;
				getch();
			}
		} else
			break;
	} while (strncmp(arg1, "quit", 1) != 0);

	wbkgd(border_console, COLOR_PAIR(2));
	wrefresh(border_console);
	touchwin(console);
	wrefresh(console);
	noecho();
}

void NcursesControl(Connection *conn, const char *file)
{
	int input;

	void mysighand(int signum) {
		if (signum == 2) {
			waddstr(body, "Catching SIGINT\nClosing DB\n");
			PREFRESH;
			DatabaseClose(conn);
			exit(1);
		}
	}

	signal(SIGINT, mysighand);

	// user input
	do {
		DisplayMode("Mode: Master");
		input = getchar();

		switch(input) {
			case '?':
				NcursesUsage();
				break;

			case 'c':
				NcursesConsole(conn, file);
				break;

			case 'e':
				RefreshdbList(conn);
				if (has_colors() == TRUE)
					wattron(body, A_REVERSE);
				PREFRESH;
				NcursesExamine(conn, file);
				break;

			case 'o':
				ReloadListing();
				touchwin(body);
				wrefresh(body);
				if (has_colors() == TRUE)
					wattron(body, A_REVERSE);
				mvwprintw(body, 0, 0, "%s", lsbuf[0]);
				PREFRESH;
				NcursesOtherdb(conn, file);
				break;

			case 'r':
				NcursesResize(conn, file);
				break;

			default:
				break;
		}	
	} while (input != 'q');
}

void NcursesExamine(Connection *conn, const char *file)
{
	int y = 0, down = 0, selection = 0; 
	int input;
	char examine_buf[MAX_DATA];
	char *arg1, *arg2;

	struct Information *rows = conn->core->db->rows;

	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);
	int *size = &(conn->core->cnf->size);

	mvwprintw(body, 0, 0, "%-2d %-10s %-10s", rows[0].index,
			rows[0].name, rows[0].phone);

	do {
		DisplayMode("Mode: Examine");
		/* debug
		   getyx(body, gety, getx);

		   mvprintw(maxy - 10, maxx - 30, 
		   "\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		   mvprintw(maxy - 7, maxx - 30, 
		   "\ngety = %d\ngetx = %d\n", gety, getx);
		   mvprintw(maxy - 10, maxx - 30, 
		   "\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		   refresh();
		 */

		PREFRESH_SCROLL;

		input = getchar();

		switch(input) {

			case 'a':
				touchwin(add);
				wrefresh(add);
				echo();
				wmove(add, 1, 1);
				wrefresh(add);
				wgetnstr(add, examine_buf, MAX_DATA);
				arg1 = strtok(examine_buf, " ");
				arg2 = strtok(NULL, " ");
				if ((arg1 == NULL) || (arg2 == NULL)) {
					DisplayError("name or phone is empty\n");
				} else {
					AddRecord(conn, &selection, arg1, arg2);
					DatabaseWrite(conn, file);
				}
				noecho();
				ClearLine(add);
				RefreshdbList(conn);
				break;

			case 'I':
				touchwin(add);
				wrefresh(add);
				echo();
				wmove(add, 1, 1);
				wrefresh(add);
				wgetnstr(add, examine_buf, MAX_DATA);
				arg1 = strtok(examine_buf, " ");
				arg2 = strtok(NULL, " ");
				if (arg1) {
					AddInsert(conn, &selection, arg1, arg2);
					DatabaseWrite(conn, file);
				}
				noecho();
				ClearLine(add);
				RefreshdbList(conn);
				break;

			case 'd':
				DeleteRecord(conn, &selection);
				DatabaseWrite(conn, file);
				RefreshdbList(conn);
				break;

			case 'D':
				DeleteInsert(conn, &selection);
				DatabaseWrite(conn, file);
				RefreshdbList(conn);
				break;

			case 'f':
				touchwin(find);
				wrefresh(find);
				echo();
				wmove(find, 1, 1);
				wrefresh(find);
				wgetnstr(find, examine_buf, MAX_DATA);
				arg1 = strtok(examine_buf, " ");
				if (arg1) {
					wmove(body, 0, 0);
					werase(body);
					DatabaseFind(conn, arg1, body);
					PREFRESH_SCROLL;
					getch();
				}
				noecho();
				ClearLine(find);
				RefreshdbList(conn);
				break;


				//case KEY_DOWN:
			case 'j':
				if (y != (maxy / 2) + 3) y++;
				wmove(body, y, 0);
				selection++;
				RefreshdbList(conn);
				if (y == (maxy / 2) + 3) down++;
				if (selection > (*size) - 1) {
					selection = 0;
					y = 0;
					down = 0;
				}
				ExamineSelection(&selection, conn);
				break;

				//case KEY_UP:
			case 'k':
				if (y != 0) y--;
				selection--;
				RefreshdbList(conn);
				if ((y == 0) && (down != 0)) down--;
				if (selection < 0) {
					selection = (*size) - 1;
					y = (*size) / 2;
					down = (*size) / 2;
				}
				ExamineSelection(&selection, conn);
				break;

			case 'r':
				touchwin(resize);
				wrefresh(resize);
				echo();
				wmove(resize, 1, 1);
				wrefresh(resize);
				wgetnstr(resize, examine_buf, MAX_DATA);
				arg1 = strtok(examine_buf, " ");
				if (arg1) {
					int i = atoi(arg1);
					if (i == *size) {
						DisplayError("database size is already that size\n");
					} else if (i <= *free_index || i <= (*delete_index) + 1) {
						DisplayError("cannot resize below valid records\n");
					} else {
						DatabaseResize(conn, &i);
						DatabaseWrite(conn, file);
					}
				}
				noecho();
				ClearLine(resize);
				RefreshdbList(conn);
				break;

			case 'A':
				DatabaseArrange(conn);
				DatabaseWrite(conn, file);
				RefreshdbList(conn);
				break;

			case 's':
				DatabaseSort(conn);
				DatabaseWrite(conn, file);
				RefreshdbList(conn);
				break;

			default:
				break;
		}

	} while (input != 'q');

	RefreshdbList(conn);
	PREFRESH_SCROLL;
}

void ExamineSelection(int *selection, Connection *conn)
{
	struct Information *rows = conn->core->db->rows;

	if (has_colors() == TRUE)
		wattron(body, A_REVERSE);
	mvwprintw(body, *selection, 0, "%-2d %-10s %-10s", rows[*selection].index,
			rows[*selection].name, rows[*selection].phone);
	touchwin(body);
	PREFRESH_SCROLL;

}

void DisplayError(const char *message)
{
	wmove(body, 0, 0);
	werase(body);
	mvwprintw(body, 0, 0, "%s", message);
	PREFRESH;
	getch();
}

void RefreshdbList(Connection *conn)
{
	wmove(body, 0, 0);
	werase(body);
	wattroff(body, A_REVERSE);
	DatabaseList(conn, body);
}

void ReloadListing()
{
	wmove(body, 0, 0);
	werase(body);
	wattroff(body, A_REVERSE);
	OtherdbListing();
}

void DisplayMode(const char *message)
{
	move(maxy - 2, 1);
	clrtoeol();
	mvprintw(maxy - 2, 1, "%s", message);
	mvprintw(maxy - 2, maxx - 17, "Press ? for help");
	refresh();
}

void ClearLine(WINDOW *win)
{
	wmove(win, 1, 1);
	wclrtoeol(win);
	wmove(win, 1, 1);
	wrefresh(win);
}

void OtherdbListing()
{
	int i = 0;
	lscount = 0;

	struct dirent **namelist;
	int n;

	n = scandir("/home/rgonzale/dev/desk/weno", &namelist, 0, alphasort);
	lscount = n;
	if (n < 0)
		perror("scandir");
	else {
		while (n--) {
			wprintw(body, "%s\n", namelist[i]->d_name);
			strncpy(lsbuf[n], namelist[n]->d_name, MAX_DATA);
			free(namelist[n]);
			i++;
		}
		free(namelist);
	}
}

void OtherdbSelection(int *selection)
{
	if (has_colors() == TRUE)
		wattron(body, A_REVERSE);
	mvwprintw(body, *selection, 0, "%s", lsbuf[*selection]);
	touchwin(body);
	PREFRESH_SCROLL;
}

void NcursesOtherdb(Connection *conn, const char *file)
{
	Connection *olddb, *newdb, *createdb;
	int y = 0, down = 0, selection = 0;
	int dbsize = 10;
	int signature, input;
	char action;
	char create_buf[MAX_DATA];
	char *arg1;
	FILE *checksig;

	olddb = conn;

	getyx(body, gety, getx);

	do {
		DisplayMode("Mode: Otherdb");
		/* debug
		   getyx(body, gety, getx);

		   mvprintw(maxy - 10, maxx - 30, 
		   "\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		   mvprintw(maxy - 7, maxx - 30, 
		   "\ngety = %d\ngetx = %d\n", gety, getx);
		   mvprintw(maxy - 10, maxx - 30, 
		   "\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		   refresh();
		 */

		prefresh(body, down, 0, 6, 2, (maxy  / 2) + 8, maxx - 3);
		input = getchar();

		switch(input) {

			case 'c':
				touchwin(create);
				wrefresh(create);
				echo();
				wmove(create, 1, 1);
				wrefresh(create);
				wgetnstr(create, create_buf, MAX_DATA);
				arg1 = strtok(create_buf, " ");

				action = 'c';

				createdb = DatabaseLoad(arg1, &action);
				DatabaseCreate(createdb, &dbsize);
				DatabaseWrite(createdb, arg1);
				DatabaseClose(createdb);

				conn = olddb;
				DisplayError("Database has been successfully created");

				noecho();
				ClearLine(create);
				ReloadListing();
				break;

			case 'e':

				checksig = fopen(lsbuf[selection], "r+");
				if (checksig == NULL) {
					DisplayError("ERROR 622: could not open file");
					break;
				}

				// check signature
				fread(&signature, sizeof(int), 1, checksig);
				if (signature != 53281) {
					DisplayError("ERROR 127: db file not weno");
					break;
				}

				DatabaseClose(conn);
				action = 'C';
				newdb = DatabaseLoad(lsbuf[selection], &action);
				DisplayError("Database has been successfully loaded, exit this mode");
				ReloadListing();
				break;

				//case KEY_DOWN:
			case 'j':
				if (y != (maxy / 2) + 3) y++;
				wmove(body, y, 0);
				selection++;
				ReloadListing();
				if (y == (maxy / 2) + 3) down++;
				if (selection > (lscount) - 1) {
					selection = 0;
					y = 0;
					down = 0;
				}
				OtherdbSelection(&selection);
				break;

				//case KEY_UP:
			case 'k':
				if (y != 0) y--;
				selection--;
				ReloadListing();
				if ((y == 0) && (down != 0)) down--;
				if (selection < 0) {
					selection = (lscount) - 1;
					y = (lscount + 1) / 2;
					down = (lscount) / 2;
				}
				OtherdbSelection(&selection);
				break;

			default:
				break;
		}
	} while (input != 'q');
	RefreshdbList(conn);
	touchwin(body);
	wrefresh(body);
	PREFRESH;
}

void NcursesResize(Connection *conn, const char *file)
{
	if (has_colors() == TRUE) {
		start_color();

		init_pair(1, COLOR_WHITE, COLOR_BLUE);
		init_pair(2, COLOR_BLACK, COLOR_CYAN);
		init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
		init_color(COLOR_WHITE, 500, 500, 500);
		init_pair(4, COLOR_BLACK, COLOR_WHITE);
		init_pair(5, COLOR_BLACK, COLOR_GREEN);
		init_pair(6, COLOR_CYAN, COLOR_CYAN);

		bkgd(COLOR_PAIR(2));
		attron(A_REVERSE);
	}

	curs_set(0);
	noecho();

	getmaxyx(stdscr, maxy, maxx);
	halfx = maxx >> 1;
	halfy = maxy >> 1;

	mvaddstr(maxy - 2, maxx - 17, "Press ? for help");

	/* debug
	   mvprintw(maxy - 4, maxx - 17, "maxy = %d", maxy);
	   mvprintw(maxy - 3, maxx - 17, "maxx = %d", maxx);
	 */

	refresh();

	// title window
	title = newwin(3 ,maxx - 2, 1, 1);
	if (title == NULL) {
		addstr("Unable to allocate memory for title window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(title, COLOR_PAIR(1));
	box(title, '|', '=');
	NcursesCenter(title, 1, banner);
	wrefresh(title);

	// body window (y size has to be tweaked by screen size)
	//border_body = newwin(((2 * maxy) / 3) - 5, maxx - 2, 5, 1);
	border_body = newwin(((2 * maxy) / 3) - 2, maxx - 2, 5, 1);
	if (border_body == NULL) {
		addstr("Unable to allocate memory for border body window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(border_body, COLOR_PAIR(1));
	box(border_body, '|', '=');
	wrefresh(border_body);

	body = newpad(maxy * 4, maxx - 2);
	if (body == NULL) {
		addstr("Unable to allocate memory for body window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(body, COLOR_PAIR(1));

	// usage window
	//border_usage = newwin((maxy / 2) - 5, maxx / 2, 
	/*border_usage = newwin((maxy / 2) + 1, maxx / 2 + 13, 
	  (maxy / 4) - 4, (maxx / 4) - 6);
	 */
	border_usage = newwin((maxy / 2) - 1, maxx / 2 + 13, 
			(maxy / 4) - 4, (maxx / 4) - 6);
	if (border_usage == NULL) {
		addstr("Unable to allocate memory for border usage window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}		

	wbkgd(border_usage, COLOR_PAIR(3));
	box(border_usage, '|', '=');

	/*usage = newwin((maxy / 2) - 1, (maxx / 2) + 11, 
	  (maxy / 4) - 3, (maxx / 4) - 5);
	 */
	usage = newwin((maxy / 2) - 3, (maxx / 2) + 11, 
			(maxy / 4) - 3, (maxx / 4) - 5);
	if (usage == NULL) {
		addstr("Unable to allocate memory for usage window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(usage, COLOR_PAIR(3));
	getmaxyx(usage, usage_maxy, usage_maxx);

	while (*help_ptr) {
		getyx(usage, usage_y, usage_x);
		if (*help_ptr == ' ')
			if (usage_maxx - usage_x < 12) waddch(usage, '\n');
		waddch(usage, *help_ptr);
		help_ptr++;
	}

	// console window
	border_console = newwin(5, maxx / 3, maxy - 8, maxx / 3);
	if (border_console == NULL) {
		addstr("Unable to allocate memory for border console window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(border_console, COLOR_PAIR(5));

	console = newwin(3, (maxx / 3) - 2, (maxy - 8 + 1), (maxx / 3) + 1);
	if (console == NULL) {
		addstr("Unable to allocate memory for console window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}	

	wbkgd(console, COLOR_PAIR(4));
	box(console, '*', '*');

	// add window
	add = SUBWINDOWS;
	if (add == NULL) {
		addstr("Unable to allocate memory for add window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}

	wbkgd(add, COLOR_PAIR(1));
	box(add, '|', '=');
	NcursesCenter(add, 0, "Add Record");

	// resize window
	resize = SUBWINDOWS;
	if (resize == NULL) {
		addstr("Unable to allocate memory for resize window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}

	wbkgd(resize, COLOR_PAIR(1));
	box(resize, '|', '=');
	NcursesCenter(resize, 0, "Resize to");

	// find window
	find = SUBWINDOWS;
	if (find == NULL) {
		addstr("Unable to allocate memory for find window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}

	wbkgd(find, COLOR_PAIR(1));
	box(find, '|', '=');
	NcursesCenter(find, 0, "Find");

	// create window
	create = SUBWINDOWS;
	if (create == NULL) {
		addstr("Unable to allocate memory for find window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}

	wbkgd(create, COLOR_PAIR(1));
	box(create, '|', '=');
	NcursesCenter(create, 0, "Add New Database");

	DatabaseList(conn, body);
	PREFRESH;
	refresh();
}

void DatabaseNcurses(Connection *conn, const char *file)
{
	initscr();

	NcursesResize(conn, file);

	NcursesControl(conn, file);

	delwin(title);
	delwin(border_body);
	delwin(body);
	delwin(border_console);
	delwin(console);
	delwin(border_usage);
	delwin(usage);
	delwin(add);
	delwin(resize);
	delwin(find);
	endwin();

	printf("weno??\n");
}
