#ifndef __GL_MATH_H__
#define __GL_MATH_H__

/// THIS WAS WRITTEN JUST TO BE DONE (HACKED)
// pretty garbage implementation from what i remember
// i simply copied gl_math files from the gl_snake project
// this needs to be rewritten to make it faster and more cleaner
// TODO

typedef struct vec3_t{
	float x;
	float y;
	float z;
}vec3_t;

typedef struct vec4_t{
	float x;
	float y;
	float z;
	float w;
}vec4_t;

typedef struct mat4_t{
	vec4_t rows[4];
}mat4_t;

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



#endif // __GL_MATH_H__