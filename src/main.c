#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "vec.h"

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

void putpixel(int x, int y, int r, int g, int b) {
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

screenpos screenpos_from_vec2f(vec2f a) {
	return (screenpos){a.x * (1 << SCREEN_FRAC_BITS), a.y * (1 << SCREEN_FRAC_BITS)};
}

// ----------------------------------------------------------------------------

vec2f screen_project(vec4f a) {
	return (vec2f){
		a.x / a.w * (SCREEN_WIDTH / 2.f) + SCREEN_WIDTH / 2.f,
		a.y / a.w * (SCREEN_HEIGHT / 2.f) + SCREEN_HEIGHT / 2.f
	};
}

void load_tri(const char *fname, float **buf, int *n_tri) {
	FILE *f = fopen(fname, "rb");
	fseek(f, 0, SEEK_END);
	unsigned long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*buf = malloc(fsize);
	if (*buf == NULL) {
		*n_tri = 0;
		fclose(f);
		return;
	}
	*n_tri = fsize / (9 * sizeof(float));

	if (fread(*buf, 1, fsize, f) < fsize)
		*n_tri = 0;
	fclose(f);
}

int main(int argc, const char **argv) {
	SDL_Window *window;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	SDL_SetWindowTitle(window, "Triangle");

	float *mesh;
	int n_tri;
	if (argc > 1) {
		load_tri(argv[1], &mesh, &n_tri);
		if (n_tri == 0) {
			printf("Empty mesh!\n");
			return -1;
		}
	}
	else {
		printf("Usage: triangle <mesh.tri>\n");
		return -1;
	}

	printf("Loaded %d triangles\n", n_tri);

	// OpenGL-style coordinate system:
	float n = 0.1f;
	float f = 1000.f;
	mat4f proj_matrix = {{
		n,    0.f,  0.f,  0.f,
		0.f,  n,    0.f,  0.f,
		0.f,  0.f, -(f + n) / (f - n), -(2.f * n * f) / (f - n),
		0.f,  0.f, -1.f,  0.f
	}};

	mat4f view_matrix;
	translatem4f(&view_matrix, (vec3f){0.f, 0.f, -3.f});

	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;
		}

		clearscreen();

		for (int t = 0; t < n_tri; ++t) {
			// Transform vertices to view space, then transform to clip space, then
			// project to screen space
			vec2f proj_verts[3];
			for (int i = 0; i < 3; ++i) {
				vec4f v = (vec4f){
					mesh[0 + 3 * (i + 3 * t)],
					mesh[1 + 3 * (i + 3 * t)],
					mesh[2 + 3 * (i + 3 * t)],
					1.f
				};
				v = mulmv4f(&view_matrix, v);
				v = mulmv4f(&proj_matrix, v);
				proj_verts[i] = screen_project(v);
			}
			// Cull backward-facing triangles
			float wind = cross2f(
				sub2f(proj_verts[1], proj_verts[0]),
				sub2f(proj_verts[2], proj_verts[0])
			);
			if (wind >= 0)
				continue;
			// Rasterise remaining screen-space triangles
			rasterise_triangle(
				screenpos_from_vec2f(proj_verts[0]),
				screenpos_from_vec2f(proj_verts[1]),
				screenpos_from_vec2f(proj_verts[2]),
				255, 0, 0
			);
		}

		vsync();
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
