#pragma once
class Renderer;

class Renderer {
public:
	Window window;
	RenderTarget target;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	QueueFamilyIndices indices;
	VkRenderPass renderPass;
	LayoutBundle layoutBundle;
	VkPipelineShaderStageCreateInfo* stages;
	VkCommandPool commandPool;
	vector<VkCommandBuffer> commandBuffers;
	vector<RenderGate*> renderGates;
	size_t currentFrame = 0;
	Renderer();
	VkGraphicsPipelineCreateInfo genPipelineInfo();
	void run();
	~Renderer();
};
