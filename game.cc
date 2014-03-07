#include "common.h"
#include "grid.h"
#include "game.h"

static grid left_grid, right_grid;

void
game_initialize()
{
	const int grid_height = GRID_ROWS*BLOCK_SIZE;
	const int grid_width = GRID_COLS*BLOCK_SIZE;

	const int base_y = (SCREEN_HEIGHT - grid_height)/2;

	left_grid.initialize(SCREEN_WIDTH/4 - grid_width/2, base_y);
	right_grid.initialize(SCREEN_WIDTH*3/4 - grid_width/2, base_y);
}

void
game_redraw()
{
	left_grid.draw();
	right_grid.draw();
}

void
game_update()
{
	left_grid.update(dpad_state);
	right_grid.update(0);
}

void
game_release()
{
}
