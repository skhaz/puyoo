extern "C" {
#include <psx.h>
}

#include "game.h"
#include "common.h"

unsigned dpad_state;

static volatile bool vblank = false;

static void
vblank_handler()
{
	vblank = true;
}

static void
dpad_state_update(void)
{
	unsigned short pad;

	PSX_ReadPad(&pad, 0);

	dpad_state = 0;
	if (pad & PAD_UP) dpad_state |= DPAD_UP;
	if (pad & PAD_DOWN) dpad_state |= DPAD_DOWN;
	if (pad & PAD_LEFT) dpad_state |= DPAD_LEFT;
	if (pad & PAD_RIGHT) dpad_state |= DPAD_RIGHT;
	if (pad & PAD_CROSS) dpad_state |= DPAD_BUTTON;
}

void
initialize(void)
{
	static unsigned int primitive_list[0x4000];

	PSX_InitEx(PSX_INIT_NOBIOS);

	GsInit();
	GsClearMem();

	GsSetVideoMode(320, 240, VMODE_PAL);

	GsSetList(primitive_list);

	SetVBlankHandler(vblank_handler);

	game_initialize();

	dpad_state = 0;
}

static void
main_loop(void)
{
	int cur_buf = 0;

	for (;;) {
		vblank = 0;
		while (!vblank)
			;

		cur_buf ^= 1;
		GsSetDispEnvSimple(0, cur_buf ? 0 : 256);
		GsSetDrawEnvSimple(0, cur_buf ? 256 : 0, 320, 240);

		dpad_state_update();
		game_update();

		GsSortCls(0, 0, 0);
		game_redraw();
		GsDrawList();

		while (GsIsDrawing())
			;
	}
}

int
main(int argc, char *argv[])
{
	initialize();
	main_loop();

	return 0;
}
