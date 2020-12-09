#ifndef SwapChainSupport_h
#define SwapChainSupport_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Window.h"
#include "Renderer.h"

class SwapChainSupport {
public:
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	bool isAdequate();
	VkSwapchainCreateInfoKHR buildInfoStruct(const Renderer& renderer, const Window& window);
	VkSurfaceFormatKHR preferredSurfaceFormat();
	VkPresentModeKHR preferredPresentMode();
	VkExtent2D preferredFrameBufferSize(const Window& window);
	uint32_t preferredImageCount();
	static SwapChainSupport queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface);
};

#endif
