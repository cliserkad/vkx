#include "StandardIncludes.h"
#include "Renderer.cpp"

// contains frame specific members for rendering
class FrameRenderer {
public:
	Renderer parent;
	VkSwapchainKHR swapchain;
	vector<VkImage> images;
	vector<VkImageView> views;
	VkRenderPass renderPass;
	VkPipelineLayout layout;
	VkPipeline pipeline;
	vector<VkFramebuffer> frameBuffers;
	vector<VkCommandBuffer> commandBuffers;

private:

};
