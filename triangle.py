#!/usr/bin/env python3

from PIL import Image

WIDTH = 320
HEIGHT = 240
SCREEN_FPSCALE = 256
screen = Image.new("RGB", (WIDTH, HEIGHT))

def putpixel(pos, colour):
	screen.putpixel(pos, colour)

def vsync():
	screen.show()

def quantise(v):
	return tuple(int(x * SCREEN_FPSCALE) for x in v)

def iter_rasterise(v0, v1, v2):
	edge_funcs = []
	# Snap vertices to subpixels. No floats!
	v0 = quantise(v0)
	v1 = quantise(v1)
	v2 = quantise(v2)
	# Edge function = (P - V0) x (V1 x V0) for P = raster <x,y>. Written in form
	# ax + by + c, where we store a, b, c
	for e in ((v0, v1), (v1, v2), (v2, v0)):
		edge_funcs.append([
			e[1][1] - e[0][1],
			e[0][0] - e[1][0],
			e[0][1] * (e[1][0] - e[0][0]) - \
			e[0][0] * (e[1][1] - e[0][1])
		])
	# Bias edge functions of top and left edges to tie-break overlaps
	for i, e in enumerate(edge_funcs):
		if (e[0] == 0 and e[1] > 0) or e[0] > 0:
			edge_funcs[i][2] -= 1
	# Iterate over bounding box. Points outside triangle have at least one negative edge function.
	bounds = lambda poly, n: range(
		min(v[n] for v in poly) // SCREEN_FPSCALE,
		max(v[n] for v in poly) // SCREEN_FPSCALE + 1
	)
	for y in bounds((v0, v1, v2), 1):
		for x in bounds((v0, v1, v2), 0):
			if all(e[0] * x * SCREEN_FPSCALE + e[1] * y * SCREEN_FPSCALE + e[2] >= 0 for e in edge_funcs):
				yield (x, y)

if __name__ == "__main__":
	for px in iter_rasterise((50, 100), (100, 220), (200, 70)):
		putpixel(px, (255, 0, 0))
	vsync()
