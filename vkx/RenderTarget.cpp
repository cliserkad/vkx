#include "RenderTarget.h"

#include <stdexcept>

#include "SwapChainSupport.h"

// contains data and settings for a Renderer
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

	RenderTarget(Renderer& renderer) {
		initSwapChain(renderer);
		initViews(renderer);
		initPipeline(renderer);
	}

	void clean(Renderer& parent) {
		for (auto framebuffer : frameBuffers) {
			vkDestroyFramebuffer(parent.device, framebuffer, nullptr);
		}
		vkDestroyPipeline(parent.device, pipeline, nullptr);
		for (auto imageView : views) {
			vkDestroyImageView(parent.device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(parent.device, swapchain, nullptr);
	}

private:

	void initSwapChain(Renderer& renderer) {
		SwapChainSupport swapChainSupport = SwapChainSupport::queryDevice(renderer.physicalDevice, renderer.window.surface);
		VkSwapchainCreateInfoKHR createInfo = swapChainSupport.buildInfoStruct(renderer, renderer.window);

		size = createInfo.imageExtent;
		if (vkCreateSwapchainKHR(renderer.device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Swapchain");
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(renderer.device, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(renderer.device, swapchain, &imageCount, images.data());
	}

	void initViews(Renderer& renderer) {
		views.resize(images.size());
		for (size_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(renderer.device, &createInfo, nullptr, &views[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void initPipeline(Renderer& renderer) {
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)size.width;
		viewport.height = (float)size.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = size;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkDynamicState dynamicStates[]{
			VK_DYNAMIC_STATE_VIEWPORT
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = sizeof(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;


		VkGraphicsPipelineCreateInfo pipelineInfo = renderer.genPipelineInfo();
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pDynamicState = &dynamicState;

		if (vkCreateGraphicsPipelines(renderer.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

	void createFrameBuffers(Renderer& renderer) {
		frameBuffers.resize(views.size());

		for (size_t i = 0; i < views.size(); i++) {
			VkImageView attachments[] = { views[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderer.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = size.width;
			framebufferInfo.height = size.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(renderer.device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void createCommandBuffers(Renderer& renderer) {
		commandBuffers.resize(frameBuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = renderer.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(renderer.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command frameBuffers");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to open command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderer.renderPass;
			renderPassInfo.framebuffer = frameBuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = size;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record to command buffer!");
			}
		}
	}

};
