#include <psx.h>

#include "gfx.h"

void
draw_rectangle(int x, int y, int w, int h, int r, int g, int b)
{
	GsRectangle rect;

	rect.r = r; rect.g = g; rect.b = b;
	rect.x = x; rect.y = y;
	rect.w = w; rect.h = h;
	rect.attribute = 0;

	GsSortRectangle(&rect);
}

void
draw_line(int x0, int y0, int x1, int y1, int r, int g, int b)
{
}
