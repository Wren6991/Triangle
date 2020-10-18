#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_FRAC_BITS 8

// ----------------------------------------------------------------------------

SDL_Renderer *renderer;

void clearscreen() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
}

int clampi(int x, int min, int max) {
	return x < min ? min : x > max ? max : x;
}

void putpixel(int x, int y, uint r, uint g, uint b) {
	SDL_SetRenderDrawColor(
		renderer,
		clampi(r, 0, 255),
		clampi(g, 0, 255),
		clampi(b, 0, 255),
		255
	);
	SDL_RenderDrawPoint(renderer, x, y);
}

void vsync() {
	SDL_RenderPresent(renderer);
}

// ----------------------------------------------------------------------------

typedef struct {
	int32_t x, y;
} screenpos;

typedef struct {
	int32_t x, y, c;
} edgefunc;

// Edge function = (P - V0) x (V1 - V0) for P = raster <x,y>. Written in form
// ax + by + c, where we store a, b, c. Points outside a convex polygon have
// at least one negative edge function.
edgefunc edgefunc_from_verts(screenpos v0, screenpos v1) {
	edgefunc e = (edgefunc){
		v1.y - v0.y,
		v0.x - v1.x,
		(v0.y * (int64_t)(v1.x - v0.x) -
		 v0.x * (int64_t)(v1.y - v0.y)) >> SCREEN_FRAC_BITS
	};
	// Bias left and top-horizontal edges to tie-break pixels which are exactly
	// on edge (we want all pixels to be covered by *exactly* one polygon)
	if ((e.x == 0 && e.y > 0) || e.x > 0)
		e.c -= 1;
	return e;
}

void rasterise_triangle(screenpos v0, screenpos v1, screenpos v2, int r, int g, int b) {
	edgefunc edges[3];
	edges[0] = edgefunc_from_verts(v0, v1);
	edges[1] = edgefunc_from_verts(v1, v2);
	edges[2] = edgefunc_from_verts(v2, v0);
	for (int y = 0; y < SCREEN_HEIGHT; ++y) { // shut up I know
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			bool inside = true;
			for (int i = 0; i < 3; ++i) {
				if (edges[i].x * x + edges[i].y * y + edges[i].c < 0) {
					inside = false;
					break;
				}
			}
			if (inside)
				putpixel(x, y, r, g, b);
		}
	}
}

// ----------------------------------------------------------------------------

typedef struct {
	float x, y;
} vec2f;

vec2f add2f(vec2f a, vec2f b) {
	return (vec2f){a.x + b.x, a.y + b.y};
}

vec2f rotate2f(vec2f a, float theta) {
	return (vec2f){
		 a.x * cosf(theta) + a.y * sinf(theta),
		-a.x * sinf(theta) + a.y * cosf(theta)
	};
}

screenpos screenpos_from_vec2f(vec2f a) {
	return (screenpos){a.x * (1 << SCREEN_FRAC_BITS), a.y * (1 << SCREEN_FRAC_BITS)};
}

// ----------------------------------------------------------------------------

int main(void) {
	SDL_Window *window;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	SDL_SetWindowTitle(window, "Triangle");


	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;
		}

		clearscreen();

		float theta = SDL_GetTicks() * 0.002f;
		vec2f centre = (vec2f){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
		vec2f v[4];
		v[0] = add2f(centre, rotate2f((vec2f){150, 0}, 0 * M_PI / 2 + theta));
		v[1] = add2f(centre, rotate2f((vec2f){120, 0}, 1 * M_PI / 2 + theta));
		v[2] = add2f(centre, rotate2f((vec2f){130, 0}, 2 * M_PI / 2 + theta));
		v[3] = add2f(centre, rotate2f((vec2f){170, 0}, 3 * M_PI / 2 + theta));

		rasterise_triangle(
			screenpos_from_vec2f(v[0]),
			screenpos_from_vec2f(v[1]),
			screenpos_from_vec2f(v[2]),
			255, 0, 0
		);

		rasterise_triangle(
			screenpos_from_vec2f(v[2]),
			screenpos_from_vec2f(v[3]),
			screenpos_from_vec2f(v[0]),
			0, 255, 0
		);

		vsync();
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
