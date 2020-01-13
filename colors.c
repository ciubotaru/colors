/**
 * File name: colors.c
 * Project name: colors, a two-player strategy game on a 2d grid written in C.
 * URL: https://github.com/ciubotaru/colors
 * Author: Vitalie Ciubotaru <vitalie at ciubotaru dot tokyo>
 * License: General Public License, version 3 or later
 * Copyright 2019
**/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <ncurses.h>

#define ROWS 20
#define COLS 40
#define COLORS 8

enum screens {
	MAIN,
	HELP,
	RESULTS,
	GAMEOVER
};

unsigned char current_player = 0;

void switch_player() {
	current_player ^= 1;
}

typedef struct node {
	unsigned char row;
	unsigned char col;
	struct node *next;
} node;

node *create_node(const unsigned char row, const unsigned char col) {
	node *new = (node *) malloc(sizeof(node));
	new->row = row;
	new->col = col;
	new->next = NULL;
	return new;
}

node *pop_node(node **queue_start) {
	if (!*queue_start) return NULL;
	node *out = *queue_start;
	*queue_start = (*queue_start)->next;
	return out;
}

void append_node(node **queue, node *new) {
	if (!*queue) {
		*queue = new;
		return;
	}
	node *current = *queue;
	while (current->next) current = current->next;
	current->next = new;
	new->next = NULL;
}

void clear_queue(node **queue) {
	node *next = NULL;
	while (*queue) {
		next = (*queue)->next;
		free(*queue);
		*queue = next;
	}
}

unsigned char grid[ROWS][COLS];

int coords[4][2];

void init_grid() {
	int i, j;
	unsigned char tmp[ROWS][COLS] = {0};
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS;j++) {
			tmp[i][j] = rand() % COLORS;
			grid[i][j] = tmp[i][j];
		}
	}
	for (i = 1; i < ROWS - 1; i++) {
		for (j = 1; j < COLS; j++) {
			if (rand() % 2) continue;
			switch(rand() % 4) {
				case 0:
					grid[i][j] = tmp[i - 1][j];
					break;
				case 1:
					grid[i][j] = tmp[i][j + 1];
					break;
				case 2:
					grid[i][j] = tmp[i + 1][j];
					break;
				case 3:
					grid[i][j] = tmp[i][j - 1];
					break;
			}
		}
	}
}

unsigned char get_color(const unsigned int row, const unsigned int col) {
	return grid[row][col];
}

void set_color(const unsigned int row, const unsigned int col, unsigned char new_color) {
	grid[row][col] = new_color;
}

void get_neighbours(const int row, const int col) {
	coords[0][0] = row - 1;
	coords[0][1] = col;
	coords[1][0] = row;
	coords[1][1] = col + 1;
	coords[2][0] = row + 1;
	coords[2][1] = col;
	coords[3][0] = row;
	coords[3][1] = col - 1;
}

unsigned int count_bits(const unsigned char ***grid) {
	unsigned int bits = 0;
	int i, j;
	for (i = 0; i < ROWS;i++) {
		for (j = 0; j < COLS; j++) {
			if ((*grid)[i][j]) bits++;
		}
	}
	return bits;
}

void traverse(const unsigned int start_row, const unsigned int start_col, unsigned char (*local_grid)[ROWS][COLS]) {
	node *queue = create_node(start_row, start_col);
	node *current_node = NULL;
	(*local_grid)[start_row][start_col] = 1;
	int i;
	while (queue) {
		current_node = pop_node(&queue);
		get_neighbours(current_node->row, current_node->col);
		for (i = 0; i < 4; i++) {
			if (coords[i][0] < 0 || coords[i][0] > ROWS - 1 || coords[i][1] < 0 || coords[i][1] > COLS - 1) continue;
			if ((*local_grid)[coords[i][0]][coords[i][1]]) continue;
			if (get_color(coords[i][0], coords[i][1]) == get_color(start_row, start_col)) {
				node *new = create_node(coords[i][0], coords[i][1]);
				append_node(&queue, new);
				(*local_grid)[coords[i][0]][coords[i][1]] = 1;
 			}
		}
		free(current_node);
	}
}

void repaint(const int start_row, const int start_col, const char new_color) {
	/* this should not happen */
	if (grid[start_row][start_col] == new_color) return;
	unsigned char local_grid[ROWS][COLS] = {0};
	traverse(start_row, start_col, &local_grid);
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (local_grid[i][j]) set_color(i, j, new_color);
		}
	}
}

int check_touch() {
	unsigned char grid1[ROWS][COLS] = {0};
	unsigned char grid2[ROWS][COLS] = {0};
	traverse(0, 0, &grid1);
	traverse(ROWS - 1, COLS - 1, &grid2);
	int i, j, k;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (grid1[i][j]) {
				get_neighbours(i, j);
				for (k = 0; k < 4; k++) {
					if (coords[k][0] < 0 || coords[k][0] > ROWS - 1 || coords[k][1] < 0 || coords[k][1] > COLS - 1) continue;
					if (grid2[coords[k][0]][coords[k][1]]) return RESULTS;
				}
			}
		}
	}
	return MAIN;
}

void print_result(WINDOW *local_win) {
	char *title = "GAME OVER";
	wclear(local_win);
	wcolor_set(local_win, 8, NULL);
	unsigned char grid1[ROWS][COLS] = {0};
	unsigned char grid2[ROWS][COLS] = {0};
	traverse(0, 0, &grid1);
	traverse(ROWS - 1, COLS - 1, &grid2);
	int i, j;
	int counter1 = 0;
	int counter2 = 0;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (grid1[i][j]) counter1++;
			if (grid2[i][j]) counter2++;
		}
	}
	i = (80 - strlen(title)) / 2;
	wprintw(local_win, "%*s\n", i, title);
//	wprintw(local_win, "Game over\n");
	wprintw(local_win, "Player 1: %i cells\n", counter1);
	wprintw(local_win, "Player 2: %i cells\n", counter2);
	if (counter1 > counter2) wprintw(local_win, "Player 1 wins!\n");
	else if (counter1 < counter2) wprintw(local_win, "Player 2 wins!\n");
	else wprintw(local_win, "A draw.\n");
	wprintw(local_win, "\n\nPress Enter to play again\n");
}

int get_input(WINDOW * window) {
	int ch = 0, ch2 = 0, input = 0;
	ch = wgetch(window);
	if (ch == 27) {
		ch2 = wgetch(window);
		if (ch2 == '[') {
			input = wgetch(window);
			if (input == 65 || input == 66 || input == 67 || input == 68)
				return input + 1000;
		}
		else return tolower(ch2);
	}
	return tolower(ch);
}

void draw_map(WINDOW *local_win) {
	wclear(local_win);
	//wattrset(local_win, A_BOLD);
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			wcolor_set(local_win, grid[i][j], NULL);
			wprintw(local_win, " ");
		}
		wprintw(local_win, "\n");
	}
	wprintw(local_win, "\n");
	unsigned char numbers[] = {1, 2, 3, 4, 5, 6, 7, 0};
	for (i = 0; i < COLORS; i++) {
		wcolor_set(local_win, 8, NULL);
		wprintw(local_win, "  %i", numbers[i]);
		wcolor_set(local_win, numbers[i], NULL);
		wprintw(local_win, " ", NULL);
	}
	wprintw(local_win, "\n");
	wcolor_set(local_win, 8, NULL);
	mvwprintw(local_win, 23, 1, "Player %i, pick your new color.\n",  current_player + 1);
}

void print_help(WINDOW *local_win) {
	wclear(local_win);
	wcolor_set(local_win, 8, NULL);
	char *title = "HELP";
	int i = (80 - strlen(title)) / 2;
	wprintw(local_win, "%*s\n", i, title);
	wprintw(local_win, "Game instructions\n [to be added]\n");
	wprintw(local_win, "\n\nPress any key\n");
}

int main() {
	srand((unsigned int) time(NULL));
	init_grid();
	initscr();
	start_color();
	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_BLACK, COLOR_RED);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_BLACK, COLOR_YELLOW);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
	init_pair(5, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(6, COLOR_BLACK, COLOR_CYAN);
	init_pair(7, COLOR_BLACK, COLOR_WHITE);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	noecho();
	curs_set(FALSE);
	WINDOW *local_win = newwin(25, 80, 0, 0);

//	print_grid();
	int new_color;
	int screen = MAIN;
	while (1) {
		switch (screen) {
			case MAIN:
				
				draw_map(local_win);
				int ch = get_input(local_win);
				unsigned char result;
				if (ch >= 0 + '0' && ch < '0' + COLORS) {
					ch -= '0';
					if (ch == (current_player ? grid[ROWS - 1][COLS - 1] : grid[0][0])) {
						//print smth
						break;
					}
					if (current_player == 1) repaint(ROWS - 1, COLS - 1, ch);
					else repaint(0, 0, ch);
					switch_player();
					screen = check_touch();
					break;
				}
				else if (ch == '?') {
					screen = HELP;
					break;
				}
				else if (ch == 'q') {
					screen = GAMEOVER;
					break;
				}
				break;
			case HELP:
				print_help(local_win);
				ch = get_input(local_win);
				screen = MAIN;
				break;
			case RESULTS:
				print_result(local_win);
				ch = get_input(local_win);
				if (ch == '\n') {
					init_grid();
					current_player = 0;
					screen = MAIN;
					break;
				}
				screen = GAMEOVER;
				break;
			case GAMEOVER:
				goto shutdown;
				break;
			default:
				goto shutdown;
		}
	}
/*
	while (!screen) {
		printf("Player %i, pick your new color: ", current_player + 1);
		//new_color = get_input();
		printf("\n");
		if (new_color == (current_player ? grid[ROWS - 1][COLS - 1] : grid[0][0])) {
			printf("This is your current color.\n");
			continue;
		}
		if (current_player == 1) repaint(ROWS - 1, COLS - 1, new_color);
		else repaint(0, 0, new_color);
		print_grid();
		screen = check_touch();
		switch_player();
	}
*/
//	print_result();
shutdown:
	curs_set(TRUE);
	echo();
	use_default_colors();
	endwin();
	return 0;
}