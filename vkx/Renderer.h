#ifndef Renderer_h
#define Renderer_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Window.h"
#include "RenderTarget.h"
#include "QueueFamilyIndices.h"
#include "RenderGate.h"
#include "LayoutBundle.h"

class Renderer {
public:
	Window window;
	RenderTarget target;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	QueueFamilyIndices indices;
	VkRenderPass renderPass;
	LayoutBundle layoutBundle;
	VkPipelineShaderStageCreateInfo* stages;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<RenderGate*> renderGates;
	size_t currentFrame = 0;
	Renderer();
	VkGraphicsPipelineCreateInfo genPipelineInfo();
	void run();
	~Renderer();
};

#endif
