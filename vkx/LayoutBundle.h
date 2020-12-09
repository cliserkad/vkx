#ifndef LayoutBundle_h
#define LayoutBundle_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Renderer.h"

class LayoutBundle {
public:
	VkPipelineLayout layout;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	LayoutBundle(Renderer* renderer);
	VkGraphicsPipelineCreateInfo genPipelineInfo(Renderer* renderer);
};

#endif
