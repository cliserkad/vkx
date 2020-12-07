#include "StandardIncludes.h"

class RenderGate {
	public:
		VkSemaphore imageAvailability;
		VkSemaphore renderCompleteness;
		VkFence occupation;
		VkDevice device;
		std::optional<uint32_t> targetImageIndex;

		~RenderGate() {
			vkDestroySemaphore(device, renderCompleteness, nullptr);
			vkDestroySemaphore(device, imageAvailability, nullptr);
			vkDestroyFence(device, occupation, nullptr);
		}

		RenderGate(VkDevice device) {
			this->device = device;
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailability) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderCompleteness) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &occupation) != VK_SUCCESS) {
				throw std::runtime_error("failed to construct RenderGate");
			}
		}

};
