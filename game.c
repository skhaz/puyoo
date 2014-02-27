#include <stdio.h>

#include "common.h"
#include "grid.h"
#include "game.h"

static struct grid left_grid, right_grid;

void
game_initialize(void)
{
	const int grid_height = GRID_ROWS*BLOCK_SIZE;
	const int grid_width = GRID_COLS*BLOCK_SIZE;

	const int base_y = (SCREEN_HEIGHT - grid_height)/2;

	grid_initialize(&left_grid, SCREEN_WIDTH/4 - grid_width/2, base_y);
	grid_initialize(&right_grid, SCREEN_WIDTH*3/4 - grid_width/2, base_y);
}

void
game_redraw(void)
{
	grid_draw(&left_grid);
	/* grid_draw(&right_grid); */
}

void
game_update(void)
{
	grid_update(&left_grid, dpad_state);
	/* grid_update(&right_grid, 0); */ /* XXX: for now */
}

void
game_release(void)
{
}
