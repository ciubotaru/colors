#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define ROWS 20
#define COLS 80
#define COLORS 10

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
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS;j++) {
			grid[i][j] = rand() % COLORS;
		}
	}
}

void print_grid() {
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			printf("%i", grid[i][j]);
		}
		printf("\n");
	}
}

int get_input() {
	int c;
	while(1) {
		c = getchar();
		if (isdigit(c) && c - '0' < COLORS) {
			return c - '0';
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
					if (grid2[coords[k][0]][coords[k][1]]) return 1;
				}
			}
		}
	}
	return 0;
}

void print_result() {
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
	printf("Game over\n");
	printf("Player 1: %i cells\n", counter1);
	printf("Player 2: %i cells\n", counter2);
	if (counter1 > counter2) printf("Player 1 wins!\n");
	else if (counter1 < counter2) printf("Player 2 wins!\n");
	else printf("A draw.\n");
}



int main() {
	srand((unsigned int) time(NULL));
	init_grid();
	print_grid();
	int new_color;
	int gameover = check_touch();
	while (!gameover) {
		printf("Player %i, pick your new color: ", current_player + 1);
		new_color = get_input();
		printf("\n");
		if (new_color == (current_player ? grid[ROWS - 1][COLS - 1] : grid[0][0])) {
			printf("This is your current color.\n");
			continue;
		}
		if (current_player == 1) repaint(ROWS - 1, COLS - 1, new_color);
		else repaint(0, 0, new_color);
		print_grid();
		gameover = check_touch();
		switch_player();
	}
	print_result();
	return 0;
}