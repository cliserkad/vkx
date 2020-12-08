#pragma once
class SwapChainSupport;

class SwapChainSupport {
public:
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;
	bool isAdequate();
	VkSwapchainCreateInfoKHR buildInfoStruct(const Renderer& renderer, const Window& window);
	VkSurfaceFormatKHR preferredSurfaceFormat();
	VkPresentModeKHR preferredPresentMode();
	VkExtent2D preferredFrameBufferSize(const Window& window);
	uint32_t preferredImageCount();
	static SwapChainSupport queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface);
};
