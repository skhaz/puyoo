#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "gfx.h"
#include "grid.h"

enum {
	MIN_CHAIN_SIZE = 3,

	DROPPING_BLOCK_TICS = 10,
	EXPLODING_BLOCK_TICS = 5,

	FALLING_BLOCK_UPDATE_INTERVAL = 3,
	FALLING_BLOCK_DROP_INTERVAL = 30,
	FALLING_BLOCK_NUM_ROTATIONS = 4,
};

static void
block_draw(int type, int x, int y)
{
	static const struct rgb {
		int r, g, b;
	} block_colors[NUM_BLOCK_TYPES - 1] = {
		{ 255, 0, 0 },
		{ 0, 255, 0 },
		{ 0, 0, 255 },
		{ 255, 255, 255 },
	};

	const rgb *color = &block_colors[type - 1];

	draw_rectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, color->r, color->g, color->b);
}

static const int offsets[FALLING_BLOCK_NUM_ROTATIONS][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };

int
grid::get_block(int r, int c) const
{
	return blocks_[r*GRID_COLS + c];
}

bool
grid::is_empty(int r, int c) const
{
	return r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && get_block(r, c) == BLOCK_EMPTY;
}

void
grid::set_block(int r, int c, int type)
{
	blocks_[r*GRID_COLS + c] = type;
}

void
falling_block::initialize()
{
	blocks_[0] = rand()%(NUM_BLOCK_TYPES - 2) + 1;
	blocks_[1] = rand()%(NUM_BLOCK_TYPES - 2) + 1;

	row_ = GRID_ROWS - 1;
	col_ = GRID_COLS/2 - 1;
	rotation_ = 0;

	input_poll_tics_ = 0;
	drop_tics_ = FALLING_BLOCK_DROP_INTERVAL;
}

void
falling_block::draw(int base_x, int base_y) const
{
	block_draw(blocks_[0], base_x + col_*BLOCK_SIZE, base_y + (GRID_ROWS - 1)*BLOCK_SIZE - row_*BLOCK_SIZE);

	block_draw(blocks_[1],
		base_x + (col_ + offsets[rotation_][1])*BLOCK_SIZE,
		base_y + (GRID_ROWS - 1)*BLOCK_SIZE - (row_ + offsets[rotation_][0])*BLOCK_SIZE);
}

bool
falling_block::can_move(const grid *g, int dr, int dc) const
{
	return
		g->is_empty(row_ + dr, col_ + dc) &&
		g->is_empty(row_ + offsets[rotation_][0] + dr, col_ + offsets[rotation_][1] + dc);
}

bool
falling_block::update(const grid *g, unsigned dpad_state)
{
	bool is_active = true;

	if (input_poll_tics_ > 0) {
		--input_poll_tics_;
	} else {
		if (dpad_state & DPAD_LEFT) {
			if (can_move(g, 0, -1)) {
				--col_;
				input_poll_tics_ = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}

		if (dpad_state & DPAD_RIGHT) {
			if (can_move(g, 0, 1)) {
				++col_;
				input_poll_tics_ = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}

		if (dpad_state & DPAD_DOWN) {
			if (can_move(g, -1, 0)) {
				--row_;
				input_poll_tics_ = FALLING_BLOCK_UPDATE_INTERVAL;
			} else {
				is_active = false;
			}
		}

		if (dpad_state & DPAD_BUTTON) {
			int next_rotation = rotation_ + 1;
			if (next_rotation == FALLING_BLOCK_NUM_ROTATIONS)
				next_rotation = 0;

			if (g->is_empty(row_ + offsets[next_rotation][0], col_ + offsets[next_rotation][1])) {
				rotation_ = next_rotation;
				input_poll_tics_ = FALLING_BLOCK_UPDATE_INTERVAL;
			}
		}
	}

	if (is_active) {
		if (drop_tics_ > 0) {
			--drop_tics_;
		} else {
			if (can_move(g, -1, 0)) {
				--row_;
				drop_tics_ = FALLING_BLOCK_DROP_INTERVAL;
			} else {
				/* can't drop */
				is_active = false;
			}
		}
	}

	return is_active;
}

void
falling_block::copy_to_grid(grid *g)
{
	g->set_block(row_, col_, blocks_[0]);

	g->set_block(
		row_ + offsets[rotation_][0],
		col_ + offsets[rotation_][1],
		blocks_[1]);
}

void
grid::initialize(int base_x, int base_y)
{
	base_x_ = base_x;
	base_y_ = base_y;

	memset(blocks_, 0, sizeof(blocks_));

	state_ = STATE_PLAYER_CONTROL;

	falling_block_.initialize();
}

void
grid::draw_blocks() const
{
	int y_offset;

	if (state_ == STATE_DROPPING_BLOCKS)
		y_offset = state_tics_*BLOCK_SIZE/DROPPING_BLOCK_TICS;
	else
		y_offset = 0;

	int x = base_x_;

	for (int c = 0; c < GRID_COLS; c++) {
		bool hanging = false;

		int y = base_y_ + (GRID_ROWS - 1)*BLOCK_SIZE;

		for (const unsigned char *p = &blocks_[c]; p < &blocks_[GRID_ROWS*GRID_COLS]; p += GRID_COLS) {
			if (*p == BLOCK_EMPTY)
				hanging = true;
			else
				block_draw(*p, x, hanging ? y + y_offset : y);

			y -= BLOCK_SIZE;
		}

		x += BLOCK_SIZE;
	}
}

void
grid::draw_background() const
{
	int y = base_y_;

	for (int i = 0; i <= GRID_ROWS; i++) {
		draw_line(base_x_, y, base_x_ + GRID_COLS*BLOCK_SIZE, y, 100, 100, 100);
		y += BLOCK_SIZE;
	}

	int x = base_x_;

	for (int i = 0; i <= GRID_COLS; i++) {
		draw_line(x, base_y_, x, base_y_ + GRID_ROWS*BLOCK_SIZE, 100, 100, 100);
		x += BLOCK_SIZE;
	}
}

int
grid::find_chain_size(int *visited, int r, int c, int type) const
{
	int rv = 0;

	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && !visited[r*GRID_COLS + c] && get_block(r, c) == type) {
		visited[r*GRID_COLS + c] = 1;

		rv = 1 +
		  find_chain_size(visited, r - 1, c, type) +
		  find_chain_size(visited, r + 1, c, type) +
		  find_chain_size(visited, r, c - 1, type) +
		  find_chain_size(visited, r, c + 1, type);
	}

	return rv;
}

void
grid::chain_explode(int r, int c, int type)
{
	if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS && get_block(r, c) == type) {
		set_block(r, c, BLOCK_EXPLODING);

		chain_explode(r - 1, c, type);
		chain_explode(r + 1, c, type);
		chain_explode(r, c - 1, type);
		chain_explode(r, c + 1, type);
	}
}

void
grid::clear_exploding_blocks()
{
	for (unsigned char *p = blocks_; p != &blocks_[GRID_ROWS*GRID_COLS]; p++)
		if (*p == BLOCK_EXPLODING)
			*p = BLOCK_EMPTY;
}

bool
grid::find_chains()
{
	static int visited[GRID_ROWS*GRID_COLS];
	memset(visited, 0, sizeof(visited));

	bool found = false;

	for (int i = 0; i < GRID_ROWS*GRID_COLS; i++) {
		if (visited[i])
			continue;

		int type = blocks_[i];

		if (type != BLOCK_EMPTY) {
			const int r = i/GRID_COLS;
			const int c = i%GRID_COLS;

			int chain_size = find_chain_size(visited, r, c, type);

			if (chain_size >= MIN_CHAIN_SIZE) {
				printf("%d chain!\n", chain_size);
				chain_explode(r, c, type);
				found = 1;
			}
		}
	}

	return found;
}

bool
grid::has_hanging_blocks() const
{
	for (const unsigned char *p = &blocks_[GRID_COLS]; p < &blocks_[GRID_ROWS*GRID_COLS]; p++) {
		if (*p != BLOCK_EMPTY && p[-GRID_COLS] == BLOCK_EMPTY)
			return true;
	}

	return false;
}

void
grid::drop_hanging_blocks()
{
	for (int c = 0; c < GRID_COLS; c++) {
		bool hanging = false;

		for (unsigned char *p = &blocks_[c]; p < &blocks_[GRID_ROWS*GRID_COLS]; p += GRID_COLS) {
			if (*p == BLOCK_EMPTY) {
				hanging = true;
			} else if (hanging) {
				p[-GRID_COLS] = *p;
				*p = BLOCK_EMPTY;
			}
		}
	}
}

void
grid::draw() const
{
	draw_background();
	draw_blocks();

	switch (state_) {
		case STATE_PLAYER_CONTROL:
			falling_block_.draw(base_x_, base_y_);
			break;

		default:
			break;
	}
}

void
grid::on_drop()
{
	if (has_hanging_blocks()) {
		state_ = STATE_DROPPING_BLOCKS;
		state_tics_ = 0;
	} else if (find_chains()) {
		state_ = STATE_EXPLODING_BLOCKS;
		state_tics_ = 0;
	} else {
		state_ = STATE_PLAYER_CONTROL;
		falling_block_.initialize();
	}
}

void
grid::update(unsigned dpad_state)
{
	switch (state_) {
		case STATE_PLAYER_CONTROL:
			if (!falling_block_.update(this, dpad_state)) {
				falling_block_.copy_to_grid(this);
				on_drop();
			}
			break;

		case STATE_EXPLODING_BLOCKS:
			if (++state_tics_ == EXPLODING_BLOCK_TICS) {
				clear_exploding_blocks();
				on_drop();
			}
			break;

		case STATE_DROPPING_BLOCKS:
			if (++state_tics_ == DROPPING_BLOCK_TICS) {
				drop_hanging_blocks();
				on_drop();
			}
			break;

		default:
			break;
	}
}
