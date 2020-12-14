#include <stdio.h>

#include "window.h"
#include "renderer.h"

#include <glad/glad.h>

#include <string.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

const char* vertex_src = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;
      
    uniform mat4 u_model;
    uniform mat4 u_projection;
    uniform mat4 u_view;

    
    out vec3 ourColor;
    out vec2 TexCoord;

      
    //out vec4 vertexColor; 

    void main()
    {

        gl_Position = u_projection*u_view*u_model*vec4(aPos, 1.0);
        ourColor = aColor;
        TexCoord = aTexCoord;
    } 


)";

const char* fragment_src = R"(
    #version 330 core
    out vec4 FragColor;
  
    in vec3 ourColor;
    in vec2 TexCoord;

    uniform sampler2D ourTexture;

    void main()
    {
         FragColor = texture(ourTexture, TexCoord);
    }

)";



float vertices[] = {
     0.5f,  0.5f, 0.0f,  0.5f, 0.0f, 0.0f,   0.0f, 1.0f,// top right
     0.5f, -0.5f, 0.0f,  0.2f, 1.0f, 0.0f,   0.0f, 0.0f,// bottom right
    -0.5f, -0.5f, 0.0f,  0.3f, 0.0f, 1.0f,   1.0f, 0.0f,// bottom left
    -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f// top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};  

struct nk_glfw g_nk_glfw = {0};
struct nk_context* g_nk_ctx;
struct nk_colorf g_nk_color;

void nk_init_ui();
void nk_do_ui();

r_texture_t bg_texture;
r_texture_t p_texture;

window_t* window;

int main()
{
	window = window_ctor("PixArt Creator", 800, 600);
	renderer_t* r = renderer_ctor();

	window->init();
	r->init(MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	r_shader_t shader = r->shader_compile(vertex_src, fragment_src);

	r->shader_set(shader);

    mat4_t ortho = ortho_projection(-1.f, 1.f, -1.f, 1.f, 1.f, 200.f);
    r->projection_set(&ortho);

    mat4_t view = mat4_identity();
    view = mat4_scale(view, vec3_construct(1.f, 1.f, 1.f));

    r->view_set(&view);
	
    r_object_t quad = r->object_load(vertices, sizeof(vertices), indices, sizeof(indices));
    

    const color_t checker_blue = (color_t){ 50, 80, 180, 100 };
	const color_t checker_white = (color_t){ 255, 255, 255, 100};

	const uint32_t bg_width = 5;
	const uint32_t bg_height = 5;
	color_t* bg_data = malloc(bg_width * bg_height * sizeof(color_t));

	// Background image
	for (uint32_t h = 0; h < bg_width; ++h)
	{
		for (uint32_t w = 0; w < bg_height; ++w)
		{
			const uint32_t idx = h * bg_width + w;

			if (h % 2 == 0) {
				bg_data[ idx ] = w % 2 == 0 ? checker_white : checker_blue;
			}
			else {
				bg_data[ idx ] = w % 2 == 0 ? checker_blue : checker_white;
			}
		}
	}

    bg_texture = r->texture_create_manual(bg_data, 5,5);
    free(bg_data);
    bg_data = malloc(128 * 128 * sizeof(color_t));

    memset(bg_data, 0, sizeof(bg_width * bg_height * sizeof(color_t)));

    

    mat4_t t = mat4_identity();

    nk_init_ui();

	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	while(1)
	{
		glClear(GL_COLOR_BUFFER_BIT);
        r->texture_delete(p_texture);


        
        p_texture = r->texture_create_manual(bg_data, 128, 128);
        r->begin();
            r->object_draw_tt(quad, &t, bg_texture);
            //r->object_draw_tt(quad, &t, p_texture);
        r->end();

        nk_do_ui();

		window->update();
	}

	window->shutdown();
	
	return 0;
}

void nk_init_ui()
{
	g_nk_color = (struct nk_colorf){0.f, 0.f, 0.f, 1.f};
    GLFWwindow* win = window->glfw_window;
    g_nk_ctx = nk_glfw3_init(&g_nk_glfw, win, 0);
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&g_nk_glfw, &atlas);
    nk_glfw3_font_stash_end(&g_nk_glfw);
}

void nk_do_ui()
{
	// Do the ui stuff
	nk_glfw3_new_frame(&g_nk_glfw);

    /* GUI */
    if (nk_begin(g_nk_ctx, "Demo", nk_rect(0, 0, 250, 600), NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(g_nk_ctx, 20, 1);
        nk_label(g_nk_ctx, "Color", NK_TEXT_LEFT);
        {
            nk_layout_row_dynamic(g_nk_ctx, 120, 1);

            g_nk_color = nk_color_picker(g_nk_ctx, g_nk_color, NK_RGBA);

            nk_layout_row_dynamic(g_nk_ctx, 25, 1);
            g_nk_color.r = nk_propertyf(g_nk_ctx, "#R:", 0, g_nk_color.r, 1.0f, 0.01f,0.005f);
            g_nk_color.g = nk_propertyf(g_nk_ctx, "#G:", 0, g_nk_color.g, 1.0f, 0.01f,0.005f);
            g_nk_color.b = nk_propertyf(g_nk_ctx, "#B:", 0, g_nk_color.b, 1.0f, 0.01f,0.005f);
            g_nk_color.a = nk_propertyf(g_nk_ctx, "#A:", 0, g_nk_color.a, 1.0f, 0.01f,0.005f);
        }

        // nk_layout_row_begin(g_nk_ctx, NK_STATIC, 2, 3);
        {
            //g_pixel_frame.paint_radius = nk_propertyi(g_nk_ctx, "Paint Radius", 0, g_pixel_frame.paint_radius, 20, 1, 0.8f);

	        // nk_layout_row_static(g_nk_ctx, 30, 80, 1);
	        if (nk_button_label(g_nk_ctx, "Save"))
	        {
	        	//pixel_frame_save_image_to_disk(pf);
	        }

	        // nk_layout_row_static(g_nk_ctx, 30, 80, 1);
	        if (nk_button_label(g_nk_ctx, "Load"))
	        {
	        	//pixel_frame_load_image_from_disk(pf);
	        }
        }
        // nk_layout_row_end(g_nk_ctx);
    }
    nk_end(g_nk_ctx);
    nk_glfw3_render(&g_nk_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}
