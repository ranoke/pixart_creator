#include "gl_math.h"

#include <stdio.h>

/// THIS WAS WRITTEN JUST TO BE DONE (HACKED)
// pretty garbage implementation from what i remember
// i simply copied gl_math files from the gl_snake project
// this needs to be rewritten to make it faster and more cleaner
// TODO



///////////
// forvard declarations
///////

/// vec3_t
vec3_t vec3_construct(float x, float y, float z);

/// vec4_t
vec4_t vec4_construct_4f(float x, float y, float z, float w);
vec4_t vec4_construct_1f(float a);

// mat4_t
mat4_t mat4_construct_1f(float a);
mat4_t mat4_construct_p(float* a);
mat4_t mat4_identity();
mat4_t mat4_translate(mat4_t mat, vec3_t v);
mat4_t mat4_scale(mat4_t mat, vec3_t s);

vec3_t mat4_get_position(mat4_t mat);

mat4_t ortho_projection(float left, float right, float bottom, float top, float near, float far);

// for debug printing
void __debug_print_mat4(mat4_t* mat);










///////////
// implementations
/////
inline
vec3_t vec3_construct(float x, float y, float z)
{
	vec3_t vec3 = {x, y, z};
	return vec3;
}

inline
vec4_t vec4_construct_1f(float a)
{
	vec4_t vec4 = {a, a, a, a};
	return vec4;
}

inline
vec4_t vec4_construct_4f(float x, float y, float z, float w)
{
	vec4_t vec4 = {x, y, z, w};
	return vec4;
}


inline
mat4_t mat4_construct_1f(float a)
{
	mat4_t mat4 = {0};
	for(int i = 0; i < 4; i++)
	{
		mat4.rows[i] = vec4_construct_1f(a);
	}
	return mat4;
}

inline
mat4_t mat4_construct_p(float* a)
{
	mat4_t mat4 = {0};
	float* mat = (float*)&mat4;
	for(int i = 0; i < 16; i++)
		mat[i] = a[i];
	return mat4;
}

inline
mat4_t mat4_identity()
{
	mat4_t mat4 = {0};
	mat4.rows[0].x = 1;
	mat4.rows[1].y = 1;
	mat4.rows[2].z = 1;
	mat4.rows[3].w = 1;
	return mat4;
}

inline
mat4_t mat4_translate(mat4_t mat, vec3_t v)
{
	mat.rows[3].x += v.x;
	mat.rows[3].y += v.y;
	mat.rows[3].z += v.z;
	return mat;
}

inline
mat4_t mat4_scale(mat4_t mat, vec3_t s)
{
	mat.rows[0].x *= s.x;
	mat.rows[1].y *= s.y;
	mat.rows[2].z *= s.z;
	return mat;
}

inline
vec3_t mat4_get_position(mat4_t mat)
{
	vec3_t vec = {0,0,0};

	vec.x = mat.rows[3].x;
	vec.y = mat.rows[3].y;
	vec.z = mat.rows[3].z;
	return vec;
}

inline
void __debug_print_mat4(mat4_t* mat)
{
	float* mat_ptr = (float*)mat;
	for(int y = 0; y < 4; y++)
	{
		for(int x = 0; x < 4; x++)
			printf("%f ", mat_ptr[(y*4) + x]);
		puts("");
	}
}

inline
mat4_t ortho_projection(float left, float right, float bottom, float top, float near, float far)
{
	mat4_t mat4 = {0};
	mat4.rows[0].x = (right-left)/2;
	mat4.rows[0].w = (left+right)/2;
	mat4.rows[1].y = (top-bottom)/2;
	mat4.rows[1].w = (top+bottom)/2;
	mat4.rows[2].z = (far-near)/(-2);
	mat4.rows[2].w = -(far+near)/2;
	mat4.rows[3].w = 1;
	return mat4;
}