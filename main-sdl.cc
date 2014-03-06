#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "game.h"

enum {
	FRAMES_PER_SECOND = 30,
	FRAME_INTERVAL = 1000/FRAMES_PER_SECOND
};

static bool running = false;

unsigned dpad_state;

static void
perror_sdl(const char *str)
{
	fprintf(stderr, "%s: %s\n", str, SDL_GetError());
}

static void
initialize_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		perror_sdl("SDL_Init");
		exit(EXIT_FAILURE);
	}

	if (SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_OPENGL) == 0) {
		perror_sdl("SDL_SetVideoMode");
		exit(EXIT_FAILURE);
	}
}

static void
release_sdl()
{
	SDL_Quit();
}

static void
initialize_opengl_state()
{
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glClearColor(0, 0, 0, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPolygonMode(GL_FRONT, GL_FILL);
}

void
initialize(void)
{
	initialize_sdl();
	initialize_opengl_state();

	game_initialize();

	dpad_state = 0;
}

static void
release(void)
{
	game_release();

	release_sdl();
}

static void
handle_keydown_event(int keysym)
{
	switch (keysym) {
		case SDLK_LEFT:
			dpad_state |= DPAD_LEFT;
			break;
		case SDLK_RIGHT:
			dpad_state |= DPAD_RIGHT;
			break;

		case SDLK_DOWN:
			dpad_state |= DPAD_DOWN;
			break;

		case SDLK_UP:
			dpad_state |= DPAD_UP;
			break;

		case SDLK_z:
			dpad_state |= DPAD_BUTTON;
			break;
	}
}

static void
handle_keyup_event(int keysym)
{
	switch (keysym) {
		case SDLK_LEFT:
			dpad_state &= ~DPAD_LEFT;
			break;
		case SDLK_RIGHT:
			dpad_state &= ~DPAD_RIGHT;
			break;

		case SDLK_DOWN:
			dpad_state &= ~DPAD_DOWN;
			break;

		case SDLK_UP:
			dpad_state &= ~DPAD_UP;
			break;

		case SDLK_z:
			dpad_state &= ~DPAD_BUTTON;
			break;
	}
}

static void
handle_events(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				handle_keydown_event(event.key.keysym.sym);
				break;

			case SDL_KEYUP:
				handle_keyup_event(event.key.keysym.sym);
				break;
		}
	}
}

static void
redraw(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	game_redraw();
	SDL_GL_SwapBuffers();
}

static void
main_loop()
{
	running = true;

	Uint32 next_frame = SDL_GetTicks() + FRAME_INTERVAL;

	while (running) {
		handle_events();
		game_update();
		redraw();

		Uint32 now = SDL_GetTicks();
		if (now < next_frame)
			SDL_Delay(next_frame - now);

		next_frame += FRAME_INTERVAL;
	}
}

int
main(int argc, char *argv[])
{
	initialize();
	main_loop();
	release();

	return 0;
}
