#ifndef RenderGate_h
#define RenderGate_h

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

#endif
