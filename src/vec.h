#ifndef _VEC_H
#define _VEC_H

#include <math.h>

// ----------------------------------------------------------------------------
// vec2f

typedef struct {
	float x, y;
} vec2f;

static inline vec2f add2f(vec2f a, vec2f b) {
	return (vec2f){a.x + b.x, a.y + b.y};
}

static inline vec2f sub2f(vec2f a, vec2f b) {
	return (vec2f){a.x - b.x, a.y - b.y};
}

static inline vec2f smul2f(vec2f a, float s) {
	return (vec2f){a.x * s, a.y * s};
}

static inline float dot2f(vec2f a, vec2f b) {
	return a.x * b.x + a.y * b.y;
}

static inline float cross2f(vec2f a, vec2f b) {
	return a.x * b.y - a.y * b.x;
}

static inline vec2f normalise2f(vec2f a) {
	return smul2f(a, 1.f / sqrtf(dot2f(a, a)));
}

static inline vec2f rotate2f(vec2f a, float theta) {
	return (vec2f){
		 a.x * cosf(theta) + a.y * sinf(theta),
		-a.x * sinf(theta) + a.y * cosf(theta)
	};
}

// ----------------------------------------------------------------------------
// vec3f

typedef struct {
	float x, y, z;
} vec3f;

static inline vec3f add3f(vec3f a, vec3f b) {
	return (vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3f sub3f(vec3f a, vec3f b) {
	return (vec3f){a.x - b.x, a.y - b.y, a.z + b.z};
}

static inline vec3f smul3f(vec3f a, float s) {
	return (vec3f){a.x * s, a.y * s, a.z * s};
}

static inline float dot3f(vec3f a, vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3f cross3f(vec3f a, vec3f b) {
	return (vec3f){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

static inline vec3f normalise3f(vec3f a) {
	return smul3f(a, 1.f / sqrtf(dot3f(a, a)));
}

// ----------------------------------------------------------------------------
// vec4f

typedef struct {
	float x, y, z, w;
} vec4f;

static inline vec4f add4f(vec4f a, vec4f b) {
	return (vec4f){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

static inline vec4f sub4f(vec4f a, vec4f b) {
	return (vec4f){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

static inline vec4f smul4f(vec4f a, float s) {
	return (vec4f){a.x * s, a.y * s, a.z * s, a.w * s};
}

static inline float dot4f(vec4f a, vec4f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// ----------------------------------------------------------------------------
// mat4f

// Row-major
typedef struct {
	float a[16];
} mat4f;

static inline void identitym4f(mat4f *Result) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			Result->a[i * 4 + j] = i == j ? 1.f : 0.f;
		}
	}
}

static inline void mulmm4f(mat4f *Result, mat4f *A, mat4f *B) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float sum = 0.f;
			for (int k = 0; k < 4; ++k)
				sum += A->a[i * 4 + k] * B->a[k * 4 + j];
			Result->a[i * 4 + j] = sum;
		}
	}
}

static inline vec4f mulmv4f(mat4f *A, vec4f b) {
	return (vec4f){
		dot4f(b, *(vec4f*)&A->a[ 0]), // you be UB
		dot4f(b, *(vec4f*)&A->a[ 4]),
		dot4f(b, *(vec4f*)&A->a[ 8]),
		dot4f(b, *(vec4f*)&A->a[12])
	};
}

#endif
