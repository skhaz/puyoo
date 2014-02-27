#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>

#include "common.h"
#include "grid.h"

enum {
	MIN_CHAIN_SIZE = 3,

	DROPPING_BLOCK_TICS = 30,

	FALLING_BLOCK_UPDATE_INTERVAL = 3,
	FALLING_BLOCK_DROP_INTERVAL = 30,
	FALLING_BLOCK_NUM_ROTATIONS = 4,
};

enum {
	BLOCK_HANGING = 0x80,
	BLOCK_FLAGS = BLOCK_HANGING
};

static void
block_draw(int type, int x, int y)
{
	GLfloat block_colors[NUM_BLOCK_TYPES - 1][4] =
		{ { 1, 0, 0, 1 },
		  { 0, 1, 0, 1 },
		  { 0, 0, 1, 1 } };

	assert(type > BLOCK_EMPTY && type < NUM_BLOCK_TYPES);

	glColor4fv(block_colors[type - 1]);

	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + BLOCK_SIZE, y);
	glVertex2f(x + BLOCK_SIZE, y + BLOCK_SIZE);
	glVertex2f(x, y + BLOCK_SIZE);
	glEnd();
}

static const int offsets[FALLING_BLOCK_NUM_ROTATIONS][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };

static inline int
grid_get_block(const struct grid *g, int r, int c)
{
	return g->blocks[r*GRID_COLS + c];
}

static inline int
grid_get_block_type(const struct grid *g, int r, int c)
{
	return grid_get_block(g, r, c) & ~BLOCK_FLAGS;
}

static inline void
grid_set_block(struct grid *g, int r, int c, int type)
{
	g->blocks[r*GRID_COLS + c] = type;
}

static void
falling_block_initialize(struct falling_block *fb)
{
	fb->blocks[0] = rand()%(NUM_BLOCK_TYPES - 1) + 1;
	fb->blocks[1] = rand()%(NUM_BLOCK_TYPES - 1) + 1;

	fb->row = GRID_ROWS - 1;
	fb->col = GRID_COLS/2 - 1;
	fb->rotation = 0;

	fb->input_poll_tics = 0;
	fb->drop_tics = FALLING_BLOCK_DROP_INTERVAL;
}

static void
falling_block_draw(const struct falling_block *fb, int base_x, int base_y)
{
	block_draw(fb->blocks[0], base_x + fb->col*BLOCK_SIZE, base_y + fb->row*BLOCK_SIZE);
	block_draw(fb->blocks[1], base_x + (fb->col + offsets[fb->rotation][1])*BLOCK_SIZE, base_y + (fb->row + offsets[fb->rotation][0])*BLOCK_SIZE);
}

static int
grid_is_empty(const struct grid *g, int r, int c)
{
	return r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && grid_get_block_type(g, r, c) == BLOCK_EMPTY;
}

static int
falling_block_can_move(struct falling_block *fb, const struct grid *g, int dr, int dc)
{
	return
	  grid_is_empty(g, fb->row + dr, fb->col + dc) &&
	  grid_is_empty(g, fb->row + offsets[fb->rotation][0] + dr, fb->col + offsets[fb->rotation][1] + dc);
}

static int
falling_block_update(struct falling_block *fb, const struct grid *g, unsigned dpad_state)
{
	int is_active = 1;

	if (fb->input_poll_tics > 0) {
		--fb->input_poll_tics;
	} else {
		if (dpad_state & DPAD_LEFT) {
			if (falling_block_can_move(fb, g, 0, -1)) {
				--fb->col;
				fb->input_poll_tics = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}

		if (dpad_state & DPAD_RIGHT) {
			if (falling_block_can_move(fb, g, 0, 1)) {
				++fb->col;
				fb->input_poll_tics = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}

		if (dpad_state & DPAD_DOWN) {
			if (falling_block_can_move(fb, g, -1, 0)) {
				--fb->row;
				fb->input_poll_tics = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}

		if (dpad_state & DPAD_BUTTON) {
			int next_rotation = fb->rotation + 1;
			if (next_rotation == FALLING_BLOCK_NUM_ROTATIONS)
				next_rotation = 0;

			if (grid_is_empty(g, fb->row + offsets[next_rotation][0], fb->col + offsets[next_rotation][1])) {
				fb->rotation = next_rotation;
				fb->input_poll_tics = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}
	}

	if (fb->drop_tics > 0) {
		--fb->drop_tics;
	} else {
		if (falling_block_can_move(fb, g, -1, 0)) {
			--fb->row;
			fb->drop_tics = FALLING_BLOCK_DROP_INTERVAL;
		} else {
			/* can't drop */
			is_active = 0;
		}
	}

	return is_active;
}

static void grid_solve_chains(struct grid *g);

void
grid_initialize(struct grid *g, int base_x, int base_y)
{
	g->base_x = base_x;
	g->base_y = base_y;

	memset(g->blocks, 0, sizeof(g->blocks));

	falling_block_initialize(&g->falling_block);

	g->state = STATE_PLAYER_CONTROL;
}

static void
grid_draw_blocks(const struct grid *g)
{
	const unsigned char *p;
	int i, y, y_offset;

	p = g->blocks;
	y = g->base_y;

	if (g->state == STATE_DROPPING_BLOCKS)
		y_offset = g->state_tics*BLOCK_SIZE/DROPPING_BLOCK_TICS;
	else
		y_offset = 0;

	for (i = 0; i < GRID_ROWS; i++) {
		int j, x;

		x = g->base_x;

		for (j = 0; j < GRID_COLS; j++) {
			unsigned char type = *p++;

			if (type != BLOCK_EMPTY) {
				int block_y = y;
				if (type & BLOCK_HANGING)
					block_y = y - y_offset;
				block_draw(type & ~BLOCK_FLAGS, x, block_y);
			}

			x += BLOCK_SIZE;
		}

		y += BLOCK_SIZE;
	}
}

static void
grid_draw_background(const struct grid *g)
{
	int i, x, y;

	glColor4f(.25, .25, .25, 1.);

	glBegin(GL_LINES);

	y = g->base_y;

	for (i = 0; i <= GRID_ROWS; i++) {
		glVertex2f(g->base_x, y);
		glVertex2f(g->base_x + GRID_COLS*BLOCK_SIZE, y);
		y += BLOCK_SIZE;
	}

	x = g->base_x;

	for (i = 0; i <= GRID_COLS; i++) {
		glVertex2f(x, g->base_y);
		glVertex2f(x, g->base_y + GRID_ROWS*BLOCK_SIZE);
		x += BLOCK_SIZE;
	}

	glEnd();
}

static int
find_chain_size(const struct grid *g, int *visited, int r, int c, int type)
{
	int rv = 0;

	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && !visited[r*GRID_COLS + c] && grid_get_block_type(g, r, c) == type) {
		visited[r*GRID_COLS + c] = 1;

		rv = 1 +
		  find_chain_size(g, visited, r - 1, c, type) +
		  find_chain_size(g, visited, r + 1, c, type) +
		  find_chain_size(g, visited, r, c - 1, type) +
		  find_chain_size(g, visited, r, c + 1, type);
	}

	return rv;
}

static void
chain_clear(struct grid *g, int r, int c, int type)
{
	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && grid_get_block_type(g, r, c) == type) {
		grid_set_block(g, r, c, BLOCK_EMPTY);

		chain_clear(g, r - 1, c, type);
		chain_clear(g, r + 1, c, type);
		chain_clear(g, r, c - 1, type);
		chain_clear(g, r, c + 1, type);
	}
}

static void
grid_find_chains(struct grid *g)
{
	int r;
	static int visited[GRID_ROWS*GRID_COLS];

	memset(visited, 0, sizeof(visited));

	for (r = 0; r < GRID_ROWS; r++) {
		int c;

		for (c = 0; c < GRID_COLS; c++) {
			if (!visited[r*GRID_COLS + r]) {
				int type = grid_get_block_type(g, r, c);

				if (type != BLOCK_EMPTY) {
					int chain_size = find_chain_size(g, visited, r, c, type);

					if (chain_size >= MIN_CHAIN_SIZE) {
						printf("%d chain!\n", chain_size);
						chain_clear(g, r, c, type);
					}
				}
			}
		}
	}
}

static int
find_lowest_chain_block(const struct grid *g, int *visited, int r, int c)
{
	int rv = -1;

	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && !visited[r*GRID_COLS + c] && grid_get_block_type(g, r, c) != BLOCK_EMPTY) {
		static const int offsets[][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };
		int i;

		visited[r*GRID_COLS + c] = 1;

		rv = r;

		for (i = 0; i < 4; i++) {
			int t = find_lowest_chain_block(g, visited, r + offsets[i][0], c + offsets[i][1]);

			if (t != -1 && t < rv)
				rv = t;
		}
	}

	return rv;
}

static void
mark_hanging_chain(struct grid *g, int r, int c)
{
	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
		int b = grid_get_block(g, r, c);

		if (b != BLOCK_EMPTY && !(b & BLOCK_HANGING)) {
			grid_set_block(g, r, c, b | BLOCK_HANGING);

			mark_hanging_chain(g, r + 1, c);
			mark_hanging_chain(g, r - 1, c);
			mark_hanging_chain(g, r, c + 1);
			mark_hanging_chain(g, r, c - 1);
		}
	}
}

static int
grid_has_hanging_blocks(struct grid *g)
{
	static int visited[GRID_ROWS*GRID_COLS];
	int r, rv = 0;

	memset(visited, 0, sizeof(visited));

	for (r = 0; r < GRID_ROWS; r++) {
		int c;

		for (c = 0; c < GRID_COLS; c++) {
			if (!visited[r*GRID_COLS + r] && grid_get_block_type(g, r, c) != BLOCK_EMPTY) {
				if (find_lowest_chain_block(g, visited, r, c) > 0) {
					mark_hanging_chain(g, r, c);
					rv = 1;
				}
			}
		}
	}

	return rv;
}

static void
grid_drop_hanging_blocks(struct grid *g)
{
	unsigned char *p;
	
	for (p = &g->blocks[GRID_COLS]; p < &g->blocks[GRID_COLS*GRID_COLS]; p++) {
		unsigned char b = *p;

		if (b & BLOCK_HANGING) {
			assert(p[-GRID_COLS] == BLOCK_EMPTY);
			p[-GRID_COLS] = b & ~BLOCK_HANGING;
			*p = BLOCK_EMPTY;
		}
	}
}

static void
grid_solve_chains(struct grid *g)
{
	grid_find_chains(g);

	if (grid_has_hanging_blocks(g)) {
		g->state = STATE_DROPPING_BLOCKS;
		g->state_tics = 0;
	} else {
		g->state = STATE_PLAYER_CONTROL;
		falling_block_initialize(&g->falling_block);
	}
}

void
grid_draw(const struct grid *g)
{
	grid_draw_background(g);
	grid_draw_blocks(g);

	switch (g->state) {
		case STATE_PLAYER_CONTROL:
			falling_block_draw(&g->falling_block, g->base_x, g->base_y);
			break;

		default:
			break;
	}
}

void
grid_update(struct grid *g, unsigned dpad_state)
{
	switch (g->state) {
		case STATE_PLAYER_CONTROL:
			if (!falling_block_update(&g->falling_block, g, dpad_state)) {
				const struct falling_block *fb = &g->falling_block;
				grid_set_block(g, fb->row, fb->col, fb->blocks[0]);
				grid_set_block(g, fb->row + offsets[fb->rotation][0], fb->col + offsets[fb->rotation][1], fb->blocks[1]);

				grid_solve_chains(g);
			}
			break;

		case STATE_DROPPING_BLOCKS:
			if (++g->state_tics == DROPPING_BLOCK_TICS) {
				grid_drop_hanging_blocks(g);

				grid_solve_chains(g);
			}
			break;

		default:
			break;
	}
}
