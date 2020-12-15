#include <stdio.h>

#include "window.h"
#include "renderer.h"

#include <glad/glad.h>
#include <cglm/cglm.h>

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

const char *vertex_src = "\n"
                         "#version 330 core\n"
                         "layout (location = 0) in vec3 aPos;\n"
                         "layout (location = 1) in vec3 aColor;\n"
                         "layout (location = 2) in vec2 aTexCoord;\n"
                         "uniform mat4 u_model;\n"
                         "uniform mat4 u_projection;\n"
                         "uniform mat4 u_view;\n"
                         "out vec3 ourColor;\n"
                         "out vec2 TexCoord;\n"
                         "void main()\n"
                         "{\n"
                         "gl_Position = u_projection*u_view*u_model*vec4(aPos, 1.0);\n"
                         "ourColor = aColor;\n"
                         "TexCoord = aTexCoord;\n"
                         "}\n";

const char *fragment_src = "\n"
                           "#version 330 core\n"
                           "out vec4 FragColor;\n"
                           "in vec3 ourColor;\n"
                           "in vec2 TexCoord;\n"
                           "uniform sampler2D ourTexture;\n"
                           "void main()\n"
                           "{\n"
                           "     FragColor = vec4(ourColor,1);\n"
                           "}\n";

float vertices[] = {
    0.5f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,   // top right
    0.5f, -0.5f, 0.0f, 0.2f, 1.0f, 0.0f, 0.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, 0.3f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom left
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f   // top left
};
unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

struct nk_glfw g_nk_glfw = {0};
struct nk_context *g_nk_ctx;
struct nk_colorf g_nk_color;

void nk_init_ui();
void nk_do_ui();

r_texture_t bg_texture;
r_texture_t p_texture;

window_t *window;

void print_mat(mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%f ", m[i][j]);
        }
        puts("");
    }
}

int main()
{
    window = window_ctor("PixArt Creator", 800, 600);
    renderer_t *r = renderer_ctor();

    window->init();
    r->init(MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    r_shader_t shader = r->shader_compile(vertex_src, fragment_src);
    r->shader_set(shader);

    mat4 ortho = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(-1, 1, -1, 1, 1, 200, ortho);
    glm_mat4_transpose(ortho);
    print_mat(ortho);

    r->projection_set(ortho);
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    r->view_set(view);
    puts("");
    print_mat(view);

    r_object_t quad = r->object_load(vertices, sizeof(vertices), indices, sizeof(indices));

    mat4 t;
    glm_mat4_identity(t);
    r->view_set(t);
    puts("");
    print_mat(t);

    nk_init_ui();
    glClearColor(0.25f, 0.25f, 0.25f, 1.f);
    while (1)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        r->begin();
        r->object_draw_tc(quad, t, (vec3){1.0f, 0.0f, 0.0f});
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
    GLFWwindow *win = window->glfw_window;
    g_nk_ctx = nk_glfw3_init(&g_nk_glfw, win, 0);
    struct nk_font_atlas *atlas;
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
            g_nk_color.r = nk_propertyf(g_nk_ctx, "#R:", 0, g_nk_color.r, 1.0f, 0.01f, 0.005f);
            g_nk_color.g = nk_propertyf(g_nk_ctx, "#G:", 0, g_nk_color.g, 1.0f, 0.01f, 0.005f);
            g_nk_color.b = nk_propertyf(g_nk_ctx, "#B:", 0, g_nk_color.b, 1.0f, 0.01f, 0.005f);
            g_nk_color.a = nk_propertyf(g_nk_ctx, "#A:", 0, g_nk_color.a, 1.0f, 0.01f, 0.005f);
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
