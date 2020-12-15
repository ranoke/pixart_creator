#include "renderer.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// like with window - don't know if its good
renderer_t* __renderer;

#define check_renderer() if(!__renderer)\
    {\
        assert(false && "no renderer");\
        return;\
    }

// just to alloc memory for renderer struct
renderer_t* renderer_ctor();
// static size for buffers
// sounds bad and not flexible, but very easy to implement
// and do not worry about it later
void __renderer_init(uint32_t vertices_size, uint32_t indices_size);
// compile shader
r_shader_t __renderer_shader_compile(const char* vertex_src, const char* fragment_src);
// create a texture from a file (.jpg image)
r_texture_t __renderer_texture_create(const char* filepath);
// create a texture from a array of bytes
r_texture_t __renderer_texture_create_manual(void* data, uint32_t width, uint32_t height);
// remove the texture from the memory
void __renderer_texture_delete(r_texture_t texture);
// for setting rendering shader
void __renderer_shader_set(r_shader_t shader);
// set projection matrix
// note the idea is to set it on start and don't worry
// about it later
void __renderer_projection_set(mat4 proj);
// sets the view transformation
// not sure is it ok, or do i have to create anoter
// functions that will allow changing position, rotation seperatly
void __renderer_view_set(mat4 view_transform);
// will simply bind vertex array and index buffer
// and use the shader program
void __renderer_begin();
// empty for now
void __renderer_end();
// load object for drawing it in the futur
// simply loads the vertex data and index data to the buffers and then returns t_object_t that contains 
// count of indices and offset for future draw
r_object_t __renderer_object_load(float* vertices, uint32_t vertices_size, uint32_t* indices, uint32_t indices_size);
// draw the loaded object with his transformation and color
void __renderer_object_draw_transform_color(r_object_t obj, mat4 transf, vec3 color);

void __renderer_object_draw_transform_texture(r_object_t obj, mat4 transf, r_texture_t texture);


renderer_t* renderer_ctor()
{
    renderer_t* r;
    r = malloc(sizeof(renderer_t));
    memset(r, 0, sizeof(renderer_t)); // just in case - do not remember to what the memory is set after malloc()
    r->init = &__renderer_init;
    r->shader_compile = &__renderer_shader_compile;
    r->texture_create = &__renderer_texture_create;
    r->texture_create_manual = &__renderer_texture_create_manual;
    r->texture_delete = &__renderer_texture_delete;
    r->shader_set = &__renderer_shader_set;
    r->projection_set = &__renderer_projection_set;
    r->view_set = &__renderer_view_set;
    r->begin = &__renderer_begin;
    r->end = &__renderer_end;
    r->object_load = &__renderer_object_load;
    r->object_draw_tc = &__renderer_object_draw_transform_color;
    r->object_draw_tt = &__renderer_object_draw_transform_texture;

    __renderer = r;
    return r;
}

void __renderer_init(uint32_t vertices_size, uint32_t indices_size)
{
    check_renderer();
    renderer_t* r = __renderer;
    uint32_t vao, vbo, ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);

    // would like to use glNamed... over this 2 function calls  but it
    // is supported in 4.5version
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, 0, GL_DYNAMIC_DRAW);


    
    // not very good thing to do from what i know
    // it's better to allow its manual managing 
    // with some function, macro, etc
    /// TODO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);  
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);  
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, 0, GL_DYNAMIC_DRAW);

    r->vertex_array = vao;
    r->vertex_buffer = vbo;
    r->index_buffer = ibo;
    r->vertex_buffer_size = vertices_size;
    r->index_buffer_size = indices_size;
    r->vertex_buffer_offset = 0;
    r->index_buffer_offset = 0;
}

// https://www.khronos.org/opengl/wiki/Shader_Compilation - reference where i got the code
r_shader_t __renderer_shader_compile(const char* vertex_src, const char* fragment_src)
{
    uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_src, 0);
    glCompileShader(vertex); 

    // check if the shader was compiled
    {
        uint32_t success = 0;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            uint32_t maxLength = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);

            char log[1024];
            glGetShaderInfoLog(vertex, maxLength, &maxLength, log);
            assert(false && "vertex shader compilation error");
            glDeleteShader(vertex);
            r_shader_t shader = {0};
            return shader;
        }
    }

    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_src, 0);
    glCompileShader(fragment);
    {
        uint32_t success = 0;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);

            char log[1024];
            glGetShaderInfoLog(vertex, maxLength, &maxLength, log);
            
            glDeleteShader(vertex);
            assert(false && "fragment compilation error");
            r_shader_t shader = {0};
            return shader;
        }
    }

    uint32_t program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);
    {
        // Note the different functions here: glGetProgram* instead of glGetShader*.
        uint32_t success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == GL_FALSE)
        {
            uint32_t maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            char log[1024];
            glGetProgramInfoLog(program, maxLength, &maxLength, log);
            

            assert(false && "failed to link shader");
            glDeleteProgram(program);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            r_shader_t shader = {0};
            return shader;
        }
    }

    // and if everyting successed we wrap the program in our struct and return
    r_shader_t shader = { program };

    return shader;
}

r_texture_t __renderer_texture_create(const char* filepath)
{
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    uint32_t width, height, nrChannels;
    unsigned char *data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        assert(false && "failed to load texture!");
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    r_texture_t t = { texture };
    return t;
}

r_texture_t __renderer_texture_create_manual(void* data, uint32_t width, uint32_t height)
{
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    r_texture_t t = { texture };
    return t;
}

void __renderer_texture_delete(r_texture_t texture)
{
    glDeleteTextures(1, &texture.texture);
}

void __renderer_shader_set(r_shader_t shader)
{
    check_renderer();
    renderer_t* r = __renderer;
    r->shader = shader;
}

void __renderer_projection_set(mat4 proj)
{
    check_renderer();
    renderer_t* r = __renderer;
    glUseProgram(r->shader.program);
    memcpy(r->projection, proj, sizeof(mat4));
    uint32_t loc = glGetUniformLocation(r->shader.program, "u_projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
}

void __renderer_view_set(mat4 view_transform)
{
    check_renderer();
    renderer_t* r = __renderer;
    glUseProgram(r->shader.program);
    memcpy(r->view_transformation, view_transform, sizeof(mat4));
    uint32_t loc = glGetUniformLocation(r->shader.program, "u_view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, view_transform);
}

void __renderer_begin()
{
    check_renderer();
    renderer_t* r = __renderer;
    glUseProgram(r->shader.program);
    glBindVertexArray(r->vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->index_buffer);
}

void __renderer_end()
{

}

r_object_t __renderer_object_load(float* vertices, uint32_t vertices_size, uint32_t* indices, uint32_t indices_size)
{
    check_renderer();
    renderer_t* r = __renderer;
    glBindBuffer(GL_ARRAY_BUFFER, r->vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, r->vertex_buffer_offset, vertices_size, vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->index_buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, r->index_buffer_offset, indices_size, indices);

    r_object_t obj = {r->index_buffer_offset, indices_size / sizeof(float) };
    r->vertex_buffer_offset += vertices_size;
    r->index_buffer_offset += indices_size;
    return obj;
}

void __renderer_object_draw_transform_color(r_object_t obj, mat4 transf, vec3 color)
{
    check_renderer();
    renderer_t* r = __renderer;
    
    
    uint32_t transf_loc = glGetUniformLocation(r->shader.program, "u_model");
    glUniformMatrix4fv(transf_loc, 1, GL_FALSE, transf);

    uint32_t color_loc = glGetUniformLocation(r->shader.program, "u_color");
    glUniform3f(color_loc, color[0], color[1], color[2]);

    glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, obj.index_offset);
}

void __renderer_object_draw_transform_texture(r_object_t obj, mat4 transf, r_texture_t texture)
{
    check_renderer();
    renderer_t* r = __renderer;
    
    glBindTexture(GL_TEXTURE_2D, texture.texture);
    
    uint32_t transf_loc = glGetUniformLocation(r->shader.program, "u_model");
    glUniformMatrix4fv(transf_loc, 1, GL_FALSE, transf);


    glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, obj.index_offset);
}
