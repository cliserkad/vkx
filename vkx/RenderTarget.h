#pragma once
class RenderTarget;

class RenderTarget {
public:
	VkSwapchainKHR swapchain;
	VkFormat format;
	VkExtent2D size;
	vector<VkImage> images;
	vector<VkImageView> views;
	VkPipeline pipeline;
	vector<VkFramebuffer> frameBuffers;
	vector<VkCommandBuffer> commandBuffers;

	RenderTarget(Renderer& renderer);
	void clean(Renderer& parent);
};
