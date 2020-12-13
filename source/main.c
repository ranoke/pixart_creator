#include <stdio.h>

#include "window.h"
#include "renderer.h"

#include <glad/glad.h>

#include <string.h>

const char* vertex_src = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
      
    uniform mat4 u_model;
    uniform mat4 u_projection;
    uniform mat4 u_view;

      
    out vec4 vertexColor; 

    void main()
    {

        gl_Position = u_projection*u_view*u_model*vec4(aPos, 1.0);
        vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
} 

)";

const char* fragment_src = R"(
    #version 330 core
    out vec4 FragColor;
      
    uniform vec3 u_color; 
    in vec4 vertexColor;

    void main()
    {
        FragColor = vec4(u_color, 1.f);
    }  

)";

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
uint32_t indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
}; 

int main()
{
	window_t* window = window_ctor("PixArt Creator", 800, 600);
	renderer_t* r = renderer_ctor();

	window->init();
	r->init(1024, 1024);
	r_shader_t shader = r->shader_compile(vertex_src, fragment_src);

	r->shader_set(shader);

    mat4_t ortho = ortho_projection(-1.f, 1.f, -1.f, 1.f, 1.f, 200.f);
    r->projection_set(&ortho);

    mat4_t view = mat4_identity();
    view = mat4_scale(view, vec3_construct(1.f/20.f, 1/15.f, 1.f));

    r->view_set(&view);
	
    r_object_t quad = r->object_load(vertices, sizeof(vertices), indices, sizeof(indices));

    mat4_t t = mat4_identity();

	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	while(1)
	{
		glClear(GL_COLOR_BUFFER_BIT);

        r->begin();
            r->object_draw_tc(quad, &t, vec3_construct(0.5f, 0.3f, 0.11f));
        r->end();

		window->update();
	}

	window->shutdown();
	
	return 0;
}