#ifndef GRID_H_
#define GRID_H_

enum {
	BLOCK_SIZE = 16,
	GRID_ROWS = 12,
	GRID_COLS = 6,
};

enum block_type {
	BLOCK_EMPTY,
	BLOCK_RED,
	BLOCK_GREEN,
	BLOCK_BLUE,
	BLOCK_EXPLODING,
	NUM_BLOCK_TYPES,
};

struct falling_block {
	int blocks[2];
	int row, col, rotation;
	int input_poll_tics;
	int drop_tics;
};

enum grid_state {
	STATE_PLAYER_CONTROL,
	STATE_EXPLODING_BLOCKS,
	STATE_DROPPING_BLOCKS,
};

struct grid {
	struct falling_block falling_block;
	unsigned char blocks[GRID_ROWS*GRID_COLS];
	int base_x, base_y;
	enum grid_state state;
	int state_tics;
};

void
grid_initialize(struct grid *g, int base_x, int base_y);

void
grid_draw(const struct grid *g);

void
grid_update(struct grid *g, unsigned dpad_state);

#endif /* GRID_H_ */
