#ifndef QueueFamilyIndices_h
#define QueueFamilyIndices_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>

class QueueFamilyIndices {
public:
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isPopulated();
	static QueueFamilyIndices queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface);
};

#endif
