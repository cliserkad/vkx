#pragma once
class RenderGate;

class RenderGate {
public:
	VkSemaphore imageAvailability;
	VkSemaphore renderCompleteness;
	VkFence occupation;
	VkDevice& device;
	std::optional<uint32_t> targetImageIndex;

	~RenderGate();

	RenderGate(VkDevice& device);

};