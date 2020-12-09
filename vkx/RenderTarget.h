#ifndef RenderTarget_h
#define RenderTarget_h

#include <vector>
#include "Renderer.h"

class RenderTarget {
public:
	VkSwapchainKHR swapchain;
	VkFormat format;
	VkExtent2D size;
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	VkPipeline pipeline;
	std::vector<VkFramebuffer> frameBuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	RenderTarget(Renderer& renderer);
	void clean(Renderer& parent);
};

#endif
