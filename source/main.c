#include <stdio.h>

#include "window.h"
#include "renderer.h"

#include <glad/glad.h>
#include <stdbool.h>
#include <cglm/cglm.h>
#include <math.h>

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

const char* frame_vertex_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;\n"
"layout(location = 1) in vec2 a_uv;\n"
"uniform mat4 u_projection;\n"
"uniform mat4 u_view;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"	gl_Position = u_projection * u_view * vec4(a_pos, 1.0);\n"
"	uv = a_uv;\n"
"}";

const char* frame_fragment_src = "\n"
"#version 330 core\n"
"uniform sampler2D u_tex;"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"	frag_color = texture(u_tex, uv);\n"
"}";


float vertices[] = {
     1.f,  1.f, 0.0f,  0.0f, 1.0f,// top right
     1.f, -1.f, 0.0f,  0.0f, 0.0f,// bottom right
    -1.f, -1.f, 0.0f,  1.0f, 0.0f,// bottom left
    -1.f,  1.f, 0.0f,  1.0f, 1.0f// top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};  

struct nk_glfw g_nk_glfw = {0};
struct nk_context* g_nk_ctx;
struct nk_colorf g_nk_color;

color_t paint_color;
int paint_radius = 4;

void nk_init_ui();
void nk_do_ui();

typedef struct pix_frame{
    vec2 tl, br;
}pix_frame;

mat4 cam_projection = GLM_MAT4_IDENTITY_INIT;
mat4 cam_transform = GLM_MAT4_IDENTITY_INIT;

r_texture_t bg_texture;
r_texture_t p_texture;

window_t *window;
renderer_t *r;

float map_range(float a1, float a2, float b1, float b2, float s)
{
    float slope = (b2 - b1) / (a2 - a1);
	return (b1 + (slope * (s - a1)));
}

bool in_range(float start, float range, float val)
{
    if( (val-start) <= range && (val-start) >= 0) return true;
    return false;
}

void pix_frame_compute_mouse_pos(pix_frame* pf, vec2 out)
{
    
	vec4 ws = {0, 0, 800, 600};
    uint32_t x, y;
    window->get_mouse_pos(&x, &y);
    printf("MOUSE: %d %d\n", x, y);
    printf("TL: %f %f\n", pf->tl[0], pf->tl[1]);
    printf("BR: %f %f\n", pf->br[0], pf->br[1]);

    if(!in_range(pf->tl[0], pf->br[0]-pf->tl[0], x)
        || !in_range(pf->tl[1], pf->br[1]-pf->tl[1], y))
    {
        out[0] = -1.f;
        out[1] = -1.f;
        return;   
    }
    printf("IN\n");

    float range_x = pf->br[0]-pf->tl[0];
    float range_y = pf->br[1]-pf->tl[1];

    float scale_x = ((x-pf->tl[0])*100)/range_x;
    float scale_y = ((y-pf->tl[1])*100)/range_y;
    printf("R: %f %f\n", scale_x/100, scale_y/100);

    out[0] = scale_x/100.f;
    out[1] = scale_y/100.f;
    
}



int32_t pix_frame_compute_idx_from_position(pix_frame* pf, float x, float y)
{
    x *= 128.f;
    y *= 128.f;
	if (x >= 128.f || y >= 128.f || x < 0 || y < 0 ) {
		return -1;
	}

    

	return (128*128)-( round(y) * 128 + round(x));
}

void calc_corners(pix_frame* pf)
{
    mat4 res;
    vec4 vp = {0,0,800,600};
    vec3 tlv = { -1.f, -1.f, 0.f};
    vec3 brv = { 1.f, +1.f, 0.f};
    glm_mat4_mul(cam_projection, cam_transform, res);

    vec3 out;
    glm_project(tlv, res, vp, out);
    
    pf->tl[0] = out[0], pf->tl[1] = out[1];

    glm_project(brv, res, vp, out);
    
    pf->br[0] = out[0], pf->br[1] = out[1];
}

int main()
{
	window = window_ctor("PixArt Creator", 800, 600);
    r = renderer_ctor();

    system("clear");
    window->init();
    r->init(MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
   
    glm_ortho(-1.f, 1.f, -1.f, 1.f, 1.f, 200.f, cam_projection);
    //glm_perspective(90.f, 800/600, 1.f, 100.f, cam_projection);
    glm_mat4_transpose(cam_projection);
    
    vec3 tr = {0, 0, -1};
    glm_translate(cam_transform, tr);
    
    

    mat4 t;
    glm_mat4_identity(t);
	
   // r_object_t quad = r->object_load(vertices, sizeof(vertices), indices, sizeof(indices));
    

    const color_t checker_blue = (color_t){ 50, 80, 180, 100 };
	const color_t checker_white = (color_t){ 255, 255, 255, 100};

	const uint32_t bg_width = 128;
	const uint32_t bg_height = 128;
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

    memset(bg_data, 0, bg_width * bg_height * sizeof(color_t));
    
    
    pix_frame pf;

    nk_init_ui();
    r_texture_t texture = r->texture_create_manual(bg_data, 128, 128);

    r_shader_t shader = r->shader_compile(frame_vertex_src, frame_fragment_src);
    r->shader_set(shader);
    r->projection_set(cam_projection);
    r->view_set(cam_transform);

    r_object_t quad = r->object_load(vertices, sizeof(vertices), indices, sizeof(indices));
       

	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	while(1)
	{
        
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,128,128, GL_RGBA, GL_UNSIGNED_BYTE, bg_data);
        uint32_t transf_loc = glGetUniformLocation(shader.program, "u_tex");
        glUniform1i(transf_loc, texture.texture);
		glClear(GL_COLOR_BUFFER_BIT);

        vec2 out;
        calc_corners(&pf);
        pix_frame_compute_mouse_pos(&pf, out);
        int idx = pix_frame_compute_idx_from_position(&pf, out[0], out[1]);
        printf("IDX: %d\n", idx);
        if(idx!=-1)
        {
            paint_color = (color_t){g_nk_color.r*255,g_nk_color.g*255,g_nk_color.b*255,g_nk_color.a*100};
            color_t col = bg_data[idx];
            printf("COLOR is %d %d %d %d", col.r, col.g, col.b, col.a);
            bg_data[idx] = paint_color;
        }
        
        
        
        r->begin();
            r->object_draw_tt(quad, &t, texture);
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

void create_circle(color_t* d, int r)
{
    color_t color_white = {255,0,0,255};
    memset(d, 0, sizeof(color_t)*r*r);
    d[0*r+r/2] = color_white;
    d[(r/2)*r+0] = color_white;
    d[((r/2)-1)*r+r-1] = color_white;
    d[(r-1)*r+r/2] = color_white;
}

void nk_do_ui()
{
	// Do the ui stuff
	nk_glfw3_new_frame(&g_nk_glfw);
    GLFWwindow* win = window->glfw_window;
    GLFWimage cursor_img;
    cursor_img.height = paint_radius;
    cursor_img.width = paint_radius;
    cursor_img.pixels = malloc(paint_radius*paint_radius*sizeof(color_t));

    create_circle(cursor_img.pixels, paint_radius);
    int hotspot_x = paint_radius/2;
    int hotspot_y = paint_radius/2;

    long cursorID = glfwCreateCursor(&cursor_img, hotspot_x , hotspot_y);

    // set current cursor
    glfwSetCursor(win, cursorID);
    


    /* GUI */
    if (nk_begin(g_nk_ctx, "Demo", nk_rect(0, 0, 250, 600), NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(g_nk_ctx, 20, 1);
        nk_label(g_nk_ctx, "Color", NK_TEXT_LEFT);
        {
            nk_layout_row_dynamic(g_nk_ctx, 120, 1);

            g_nk_color = nk_color_picker(g_nk_ctx, g_nk_color, NK_RGBA);

            nk_layout_row_dynamic(g_nk_ctx, 25, 1);
            g_nk_color.r = nk_propertyf(g_nk_ctx, "#R:", 0, g_nk_color.r, 1, 1,1);
            g_nk_color.g = nk_propertyf(g_nk_ctx, "#G:", 0, g_nk_color.g, 1, 1,1);
            g_nk_color.b = nk_propertyf(g_nk_ctx, "#B:", 0, g_nk_color.b, 1, 1,1);
            g_nk_color.a = nk_propertyf(g_nk_ctx, "#A:", 0, g_nk_color.a, 1, 1,1);
        }

        // nk_layout_row_begin(g_nk_ctx, NK_STATIC, 2, 3);
        {
            paint_radius = nk_propertyi(g_nk_ctx, "Paint Radius", 0, paint_radius, 20, 1, 0.8f);

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
