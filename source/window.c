#include "window.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

window_t* window_ctor(const char* title, uint32_t width, uint32_t height);
void __window_init();
void __window_update();
void __window_shutdown();
void __window_get_mouse_pos(uint32_t* x, uint32_t* y);

// no idea is this a good idea, but i want to return pointer to this after window
// initialization
window_t __window = {0};
uint32_t mouse_x, mouse_y;



window_t* window_ctor(const char* title, uint32_t width, uint32_t height)
{
	strcpy(__window.title, title);
	__window.width = width;
	__window.height = height;

	__window.init = &__window_init;
	__window.update = &__window_update;
	__window.shutdown = &__window_shutdown;
	__window.get_mouse_pos = &__window_get_mouse_pos;

	return &__window;
}

void __window_init()
{
	if(!glfwInit())
	{
		assert(false && "Failed to init glfw");
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	__window.glfw_window = glfwCreateWindow(__window.width, __window.height, __window.title, 0, 0);

    if(!__window.glfw_window)
    {
        assert(false && "export MESA_GLSL_VERSION_OVERRIDE=410; export MESA_GL_VERSION_OVERRIDE=4.1COMPAT");
        return;
    }
    glfwMakeContextCurrent(__window.glfw_window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(1);

}

void __window_update()
{
	double x, y;
	glfwGetCursorPos(__window.glfw_window, &x, &y);
	mouse_y = (uint32_t)y;
	mouse_x = (uint32_t)x;
	glfwPollEvents();
	glfwSwapBuffers(__window.glfw_window);
}

void __window_shutdown()
{
	glfwDestroyWindow(__window.glfw_window);
	glfwTerminate();
}

void __window_get_mouse_pos(uint32_t* x, uint32_t* y)
{
	if(    mouse_x < 0 
		|| mouse_y < 0 
		|| mouse_x > __window.width 
		|| mouse_y > __window.height) return;
	*x = mouse_x;
	*y = mouse_y;
}


