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

// no idea is this a good idea, but i want to return pointer to this after window
// initialization
window_t __window = {0};



window_t* window_ctor(const char* title, uint32_t width, uint32_t height)
{
	strcpy(__window.title, title);
	__window.width = width;
	__window.height = height;

	__window.init = &__window_init;
	__window.update = &__window_update;
	__window.shutdown = &__window_shutdown;

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
	glfwPollEvents();
	glfwSwapBuffers(__window.glfw_window);
}

void __window_shutdown()
{
	glfwDestroyWindow(__window.glfw_window);
	glfwTerminate();
}


