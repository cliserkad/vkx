#ifndef Window_h
#define Window_h

#include "Renderer.h"

class Window {
public:
	GLFWwindow* window;
	VkSurfaceKHR surface;
	Window(Renderer* renderer);
};

#endif
