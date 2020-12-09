#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "Renderer.h"

class Window {
public:
	// default window size of 720p
	const uint32_t DEFAULT_WINDOW_WIDTH = 1280;
	const uint32_t DEFAULT_WINDOW_HEIGHT = 720;

	GLFWwindow* window;
	VkSurfaceKHR surface;

	Window(Renderer* renderer) {
		// create glfw window
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Renderer", nullptr, nullptr);

		// link glfw to vulkan
		if (glfwCreateWindowSurface(renderer->instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to link vulkan to glfw");
		}
	}

};
