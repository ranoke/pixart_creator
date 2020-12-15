#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <stdint.h>
#include <cglm/cglm.h>

// i decided to wrap it in its own class to make
// it much easier to think about
typedef struct r_shader_t{
    uint32_t program;
}r_shader_t;


typedef struct r_texture_t{
    uint32_t texture;
}r_texture_t;


// for rendering specific object we only need this two values
// the offset in the IBO(index buffer object) and the count
// of indecies that we will draw
typedef struct r_object_t{
    uint32_t index_offset;
    uint32_t count;
}r_object_t;


typedef struct renderer_t{
    void (* init)(uint32_t, uint32_t);
    r_shader_t (* shader_compile)(const char*, const char*);
    r_texture_t (* texture_create)(const char*);
    r_texture_t (* texture_create_manual)(void*, uint32_t, uint32_t);
    void (* texture_delete)(r_texture_t);
    void (* shader_set)(r_shader_t);
    void (* projection_set)(mat4);
    void (* view_set)(mat4);
    void (* begin)();
    void (* end)();
    r_object_t (* object_load)(float*, uint32_t, uint32_t*, uint32_t);
    void (* object_draw_tc)(r_object_t, mat4, vec3);
    void (* object_draw_tt)(r_object_t, mat4, r_texture_t);
    

    r_shader_t shader; 
    mat4 projection;
    mat4 view_transformation;

    uint32_t vertex_array;
    uint32_t vertex_buffer;
    uint32_t index_buffer;
    uint32_t vertex_buffer_offset;
    uint32_t index_buffer_offset;
    uint32_t vertex_buffer_size;
    uint32_t index_buffer_size;
}renderer_t;

renderer_t* renderer_ctor();


#endif // __RENDERER_H__