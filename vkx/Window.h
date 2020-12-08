#pragma once
class Window;

class Window {
public:
	GLFWwindow* window;
	VkSurfaceKHR surface;
	Window(Renderer* renderer);
};
