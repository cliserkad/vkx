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

class QueueFamilyIndices {
	public:
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isPopulated() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}

		static QueueFamilyIndices queryDevice(VkPhysicalDevice& device, VkSurfaceKHR& surface) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
				}

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

				if (presentSupport) {
					indices.presentFamily = i;
				}

				if (indices.isPopulated()) {
					break;
				}

				i++;
			}

			return indices;
		}
};
