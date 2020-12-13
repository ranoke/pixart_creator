#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdint.h>

typedef struct window_t{
	void (* init)();
	void (* update)();
	void (* shutdown)();

	char title[255];
	uint32_t width;
	uint32_t height;
	void* glfw_window;
}window_t;

window_t* window_ctor(const char* title, uint32_t width, uint32_t height);

#endif //__WINDOW_H__