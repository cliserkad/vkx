#pragma once
class LayoutBundle;

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
