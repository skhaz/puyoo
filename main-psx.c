#include <psx.h>

unsigned dpad_state;

static volatile int vblank = 0;

static void
vblank_handler(void)
{
	vblank = 1;
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
