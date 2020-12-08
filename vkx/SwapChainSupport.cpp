#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>

#include "Renderer.h"
#include "Window.h"
#include "SwapChainSupport.h"
#include "RenderGate.h"
#include "QueueFamilyIndices.h"
#include "ShaderModule.h"
#include "LayoutBundle.h"
#include "RenderTarget.h"

using namespace std;

class SwapChainSupport {
	public:
		VkSurfaceCapabilitiesKHR capabilities;
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> presentModes;
		bool isAdequate() {
			return !formats.empty() && !presentModes.empty();
		}
		VkSwapchainCreateInfoKHR buildInfoStruct(const Renderer& renderer, const Window& window) {
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = renderer.surface;
			createInfo.minImageCount = preferredImageCount();
			createInfo.imageFormat = preferredSurfaceFormat().format;
			createInfo.imageColorSpace = preferredSurfaceFormat().colorSpace;
			createInfo.imageExtent = preferredFrameBufferSize(window);
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // simple usage
			createInfo.preTransform = capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = preferredPresentMode();
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			uint32_t queueFamilyIndicesArray[] = {
				renderer.indices.graphicsFamily.value(),
				renderer.indices.presentFamily.value()
			};
			if (renderer.indices.graphicsFamily != renderer.indices.presentFamily) {
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
			}
			else
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			return createInfo;
		}
		VkSurfaceFormatKHR preferredSurfaceFormat() {
			// prefer RGBA 8 bit encoding in SRGB colorspace
			for (const auto& format : formats) {
				if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
					return format;
			}
			return formats[0];
		}
		VkPresentModeKHR preferredPresentMode() {
			// prefer "triple-buffer" vsync
			for (const auto& presentMode : presentModes) {
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					return presentMode;
			}
			// vsync
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		VkExtent2D preferredFrameBufferSize(const Window& window) {
			if (capabilities.currentExtent.width != UINT32_MAX) {
				return capabilities.currentExtent;
			} else {
				int width, height = 0;
				glfwGetFramebufferSize(window.window, &width, &height);
				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};
				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
				return actualExtent;
			}
		}
		uint32_t preferredImageCount() {
			uint32_t imageCount = capabilities.minImageCount + 1;
			if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
				imageCount = capabilities.maxImageCount;
			return imageCount;
		}
		static SwapChainSupport queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface) {
			SwapChainSupport details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}
};
