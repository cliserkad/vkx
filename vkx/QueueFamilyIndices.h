#pragma once

class QueueFamilyIndices {
public:
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isPopulated();
	static QueueFamilyIndices queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface);
};


