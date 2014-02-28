#include <GL/gl.h>

#include "gfx.h"

void
draw_rectangle(int x, int y, int w, int h, int r, int g, int b)
{
	glColor4ub(r, g, b, 255);

	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x + w, y + h);
	glVertex2i(x, y + h);
	glEnd();
}

void
draw_line(int x0, int y0, int x1, int y1, int r, int g, int b)
{
	glColor4ub(r, g, b, 255);

	glBegin(GL_LINES);
	glVertex2i(x0, y0);
	glVertex2i(x1, y1);
	glEnd();
}
