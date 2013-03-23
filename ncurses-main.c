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
#include "prototypes.h"

#define MAX_DATA 32

int up, down;
int maxy, maxx, halfy, halfx;
int usage_y, usage_x, usage_maxy, usage_maxx;

WINDOW *title, *border_body, *body, *border_console, *console,
	   *border_usage, *usage, *add, *resize;

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
	touchwin(border_usage);
	wrefresh(border_usage);
	touchwin(usage);
	wrefresh(usage);
	getch();
	prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
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
		wmove(body, 0, 0);
		werase(body);
		DatabaseList(conn, body);
		prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);

		wgetnstr(console, console_buf, MAX_DATA);
		arg1 = strtok(console_buf, " ");
		arg2 = strtok(NULL, " ");
		arg3 = strtok(NULL, " ");
		arg4 = strtok(NULL, " ");

		wmove(console, 1, 1);
		wclrtoeol(console);
		wmove(console, 1, 1);
		wrefresh(console);

		if (arg1 != NULL) {

			if (strncmp(arg1, "add", 1) == 0) {
				if (arg4 == NULL) {
					if ((arg2 == NULL) || (arg3 == NULL)) {
						wmove(body, 0, 0);
						werase(body);
						waddstr(body, "name or phone are empty\n");
						prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
						getch();
					} else
						AddRecord(conn, NULL, arg2, arg3);
				} else {
					int i = atoi(arg2);
					if (i > *size || i < 0) {
						wmove(body, 0, 0);
						werase(body);
						waddstr(body, "index out of bounds\n");
						waddstr(body, "try again\n");
						getch();
					} else {
						if ((arg3 == NULL) || (arg4 == NULL)) {
							wmove(body, 0, 0);
							werase(body);
							waddstr(body, "name or phone are empty\n");
							prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
							getch();
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
						wmove(body, 0, 0);
						werase(body);
						waddstr(body, "index out of bounds\n");
						waddstr(body, "try again\n");
						prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
						getch();
					} else {
						AddInsert(conn, &i, arg3, arg4);
					}
				}
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "delete", 1) == 0) {
				if (*free_index == 0 && *delete_index == 0) {
					wmove(body, 0, 0);
					werase(body);
					waddstr(body, "database is already empty\n");
					waddstr(body, "you can only add a record\n");
					prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
					getch();
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
					wmove(body, 0, 0);
					werase(body);
					waddstr(body, "database is already empty\n");
					waddstr(body, "you can only add a record\n");
					prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
					getch();
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
					wmove(body, 0, 0);
					werase(body);
					waddstr(body, "database size is already that size\n");
					waddstr(body, "choose another size\n");
					prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
					getch();
				} else if (i <= *free_index || i <= (*delete_index) + 1) {
					wmove(body, 0, 0);
					werase(body);
					waddstr(body, "cannot resize below valid records\n");
					waddstr(body, "choose a larger size\n");
					prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
					getch();
				} else {
					DatabaseResize(conn, &i);
					DatabaseWrite(conn, file);
				}
			}
			if (strncmp(arg1, "sort", 1) == 0) {
				DatabaseSort(conn);
				DatabaseWrite(conn, file);
			}
			if (strncmp(arg1, "find", 1) == 0) {
				wmove(body, 0, 0);
				werase(body);
				DatabaseFind(conn, arg2, body);
				prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
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

	struct Information *rows = conn->core->db->rows;

	void mysighand(int signum) {
		if (signum == 2) {
			waddstr(body, "Catching SIGINT\nClosing DB\n");
			prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
			DatabaseClose(conn);
			exit(1);
		}
	}

	signal(SIGINT, mysighand);

	// user input
	do {
		input = getchar();

		switch(input) {
			case '?':
				NcursesUsage();
				break;

			case 'c':
				NcursesConsole(conn, file);
				break;

			case 'e':
				if (has_colors() == TRUE)
					wattron(body, A_REVERSE);
				mvwprintw(body, 0, 0, "%d %s %s", rows[0].index,
						rows[0].name, rows[0].phone);
				prefresh(body, 0, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
				NcursesExamine(conn, file);
				break;

			default:
				break;
		}	
	} while (input != 'q');
}

void NcursesExamine(Connection *conn, const char *file)
{
	int y, x;
	int gety, getx;
	gety = 0;
	getx = 0;
	y = 0;
	x = 0;
	down = 0;
	up = 0;
	int selection;
	int input;
	char examine_buf[MAX_DATA];

	char *arg1, *arg2;

	selection = 0;

	int *free_index = &(conn->core->cnf->free_index);
	int *delete_index = &(conn->core->cnf->delete_index);
	int *size = &(conn->core->cnf->size);
	do {
		getyx(body, gety, getx);

		// debug
		mvprintw(maxy - 12, maxx - 30,
				"\nfree_index = %d\n", *free_index);
		mvprintw(maxy - 11, maxx - 30,
				"\ndelete_index = %d\n", *delete_index);
		mvprintw(maxy - 10, maxx - 30, 
				"\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		mvprintw(maxy - 7, maxx - 30, 
				"\ngety = %d\ngetx = %d\n", gety, getx);
		mvprintw(maxy - 10, maxx - 30, 
				"\ny = %d\nx = %d\ndown = %d\n", y, x, down);
		refresh();
		// end of debug

		prefresh(body, down, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
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
				if (arg1) {
					AddRecord(conn, &selection, arg1, arg2);
					DatabaseWrite(conn, file);
				}
				noecho();
				wmove(add, 1, 1);
				wclrtoeol(add);
				wmove(add, 1, 1);
				wrefresh(add);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
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
				wmove(add, 1, 1);
				wclrtoeol(add);
				wmove(add, 1, 1);
				wrefresh(add);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				break;

			case 'd':
				DeleteRecord(conn, &selection);
				DatabaseWrite(conn, file);
				wmove(body, 0, 0);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				break;

			case 'D':
				DeleteInsert(conn, &selection);
				DatabaseWrite(conn, file);
				wmove(body, 0, 0);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				break;

				//case KEY_DOWN:
			case 'j':
				if (y != (maxy / 2) + 3) y++;
				wmove(body, y, 0);
				selection++;
				wmove(body, 0, 0);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				if (y == (maxy / 2) + 3) down++;
				if (selection > (*size) - 1) {
					selection = 0;
					y = 0;
					down = 0;
				}
				NcursesSelection(&selection, conn);
				break;

				//case KEY_UP:
			case 'k':
				if (y != 0) y--;
				selection--;
				wmove(body, 0, 0);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				if (y == 0) down--;
				if (selection < 0) {
					selection = (*size) - 1;
					y = (*size) / 2;
					down = (*size) / 2;
				}
				NcursesSelection(&selection, conn);
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
						wmove(body, 0, 0);
						werase(body);
						waddstr(body, "database size is already that size\n");
						waddstr(body, "choose another size\n");
						prefresh(body, down, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
						getch();
					} else if (i <= *free_index || i <= (*delete_index) + 1) {
						wmove(body, 0, 0);
						werase(body);
						waddstr(body, "cannot resize below valid records\n");
						waddstr(body, "choose a larger size\n");
						prefresh(body, down, 0, 6, 2, (maxy / 2) + 8, maxx - 3);
						getch();
					} else {
						DatabaseResize(conn, &i);
						DatabaseWrite(conn, file);
					}
				}
				noecho();
				wmove(resize, 1, 1);
				wclrtoeol(resize);
				wmove(resize, 1, 1);
				wrefresh(resize);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				break;

			case 's':
				DatabaseSort(conn);
				DatabaseWrite(conn, file);
				wmove(body, 0, 0);
				werase(body);
				wattroff(body, A_REVERSE);
				DatabaseList(conn, body);
				break;

			default:
				break;
		}

	} while (input != 'q');

	wmove(body, 0, 0);
	werase(body);
	wattroff(body, A_REVERSE);
	DatabaseList(conn, body);
	prefresh(body, down, 0, 6, 2, (maxy /2) + 8, maxx - 3);
}

void NcursesSelection(int *selection, Connection *conn)
{
	struct Information *rows = conn->core->db->rows;

	if (has_colors() == TRUE)
		wattron(body, A_REVERSE);
	mvwprintw(body, *selection, 0, "%d %s %s", rows[*selection].index,
			rows[*selection].name, rows[*selection].phone);
	touchwin(body);
	prefresh(body, down, 0, 6, 2, (maxy / 2) + 8, maxx - 3);

}

void DatabaseNcurses(Connection *conn, const char *file)
{
	char banner[] = "weno shell v0.1 by Tristan Gonzalez - Copyright 2013";
	char help[] = {"[Master modes]\n"
		" c - console, type in commands\n"
			" e - examine, add/delete based on highlighted index\n"
			" q = quit\n\n"

			"[Commands]\n "

			"a - add_record, console mode - <index> <name> <phone>, if index is "
			"omitted record added to next available index\n "
			"d - delete_record, <index>, if index omitted last record "
			"deleted\n "
			"D - delete_insert, <index>, delete and shift\n "
			"I - insert, <index>, insert record and shift\n "
			"f - find, <name>\n "
			"q - quit, exit mode\n "
			"r - resize <newsize>\n "
			"s - sort"};

	char *help_ptr = help;

	/*int i;
	  fprintf(stderr, "Initializing Ncurses");
	  for (i = 0; i < 29; i++) {
	  napms(20);
	  fprintf(stderr, ".");
	  }
	  fprintf(stderr, "\nNcurses Engine Loaded\n");
	  napms(500);
	 */

	initscr();

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
	mvprintw(maxy - 4, maxx - 17, "maxy = %d", maxy);
	mvprintw(maxy - 3, maxx - 17, "maxx = %d", maxx);
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
	border_usage = newwin((maxy / 2) - 5, maxx / 2, 
			maxy / 4, maxx / 4);
	if (border_usage == NULL) {
		addstr("Unable to allocate memory for border usage window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}		

	wbkgd(border_usage, COLOR_PAIR(3));
	box(border_usage, '|', '=');

	usage = newwin((maxy / 2) - 7, (maxx / 2) - 2, 
			(maxy / 4) + 1, (maxx / 4) + 1);
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
			if (usage_maxx - usage_x < 6) waddch(usage, '\n');
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
	add = newwin(3, (maxx/3) - 2, maxy / 4, maxx / 4);
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
	resize = newwin(3, (maxx/3) - 2, maxy / 4, maxx / 4);
	if (resize == NULL) {
		addstr("Unable to allocate memory for resize window");
		DatabaseClose(conn);
		endwin();
		exit(1);
	}

	wbkgd(resize, COLOR_PAIR(1));
	box(resize, '|', '=');
	NcursesCenter(resize, 0, "Resize to");

	DatabaseList(conn, body);
	prefresh(body, 0, 0, 6, 2, (maxy  / 2) + 8, maxx - 3);
	refresh();

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
	endwin();

	printf("weno??\n");
}
