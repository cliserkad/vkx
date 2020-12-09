#include "Renderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>
#include <fstream>
#include <map>
#include <set>

#include "ShaderModule.h"
#include "SwapChainSupport.h"


const int CONCURRENT_RENDER_FRAMES = 2;

// list of necessary vulkan extensions for rendering
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// get the validation layers defined in the Vulkan SDK
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
// turn on validation layers if compiled in debug mode
#ifdef NDEBUG
const bool debugMode = false;
#else
const bool debugMode = true;
#endif

class Renderer {
public:
	Window window;
	RenderTarget target;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
	Renderer() {
		createInstance();
		registerDevice();
		createLogicalDevice();
		createRenderGates();
		initRenderPass();
		initShaderStages();
		createCommandPool();
		window = Window(this);
		target = RenderTarget(*this);
	}
	void run() {
		while (!glfwWindowShouldClose(window.window)) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}
	VkGraphicsPipelineCreateInfo genPipelineInfo() {
		return layoutBundle.genPipelineInfo(this);
	}
	~Renderer() {
		destruct();
	}

private:
	void initRenderPass() {
		VkAttachmentDescription colorAttachment{};
		SwapChainSupport support = SwapChainSupport::queryDevice(physicalDevice, window.surface);

		colorAttachment.format = support.preferredSurfaceFormat().format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}
	void initShaderStages() {
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");
		ShaderModule vertShaderModule(vertShaderCode, device);
		ShaderModule fragShaderModule(fragShaderCode, device);

		VkPipelineShaderStageCreateInfo stages[] = {
		   vertShaderModule.shaderCreateInfo(false),
		   fragShaderModule.shaderCreateInfo(true)
		};

		this->stages = stages;
	}
	void createRenderGates() {
		for (size_t i = 0; i < CONCURRENT_RENDER_FRAMES; i++) {
			renderGates.push_back(new RenderGate(device));
		}
	}
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::queryDevice(physicalDevice, window.surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	
	void createLogicalDevice() {
		indices = QueueFamilyIndices::queryDevice(physicalDevice, window.surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (debugMode) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
	void registerDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("no gpu in this machine has vulkan support");
		} else {
			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
			std::vector<VkPhysicalDevice> candidates;
			for (const auto& device : devices) {
				// if current device is valid, add to candidates
				if(isDeviceSuitable(device)) {
					candidates.push_back(device);
				}
			}
			if(candidates.size() == 0)
				throw std::runtime_error("no gpu in this machine has required vulkan features");
		
			std::multimap<int64_t, VkPhysicalDevice> ratedCandidates;
			for (const auto& candidate : candidates) {
				ratedCandidates.insert(std::make_pair(rateDevice(candidate), candidate));
			}
			physicalDevice = ratedCandidates.rbegin()->second;
			std::cout << "vulkan device rating:" << ratedCandidates.rbegin()->first << "\n";
		}
	}
	int rateDevice(VkPhysicalDevice device) {
		if(isDeviceSuitable(device) == false)
			return 0;
		else {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);
			int score = 1;

			// max texture size
			score *= props.limits.maxImageDimension2D + 1;

			switch (props.deviceType) {
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					score *= 1;
					break;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					score *= 2;
					break;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					score *= 3;
					break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					score *= 4;
					break;
				default:
					score *= 1;
			}

			return score;
		}
	}
	bool isDeviceSuitable(VkPhysicalDevice device) {
		return QueueFamilyIndices::queryDevice(device, window.surface).isPopulated() && isDeviceExtended(device) && SwapChainSupport::queryDevice(device, window.surface).isAdequate();
	}
	bool isDeviceExtended(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}
	void createInstance() {
		ensureValidationSuccess();

		// fill a struct that contains information about the app
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "None";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// fill a struct that informs vulkan of glfw's extensions & our app info
		VkInstanceCreateInfo creationInfo{};
		creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creationInfo.pApplicationInfo = &appInfo;
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		creationInfo.enabledExtensionCount = glfwExtensionCount;
		creationInfo.ppEnabledExtensionNames = glfwExtensions;
		if (debugMode) {
			creationInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			creationInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			creationInfo.enabledLayerCount = 0;
		}
		VkResult result = vkCreateInstance(&creationInfo, nullptr, &instance);
		if (result == VK_SUCCESS) {
			std::cout << "created vulkan instance\n";
		} else {
			throw std::runtime_error("failed to create vulkan instance. error: " + result);
		}

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}
	}
	void ensureValidationSuccess() {
		if(debugMode) {
			if(!isValidationAvailable())
				throw std::runtime_error("vulkan validation layers requested, but aren't available");
			else
				std::cout << "vulkan validation is enabled\n";
		}
	}
	bool isValidationAvailable() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					goto nextLayer;
				}
			}
			return false;
			nextLayer:
			continue;
		}
		return true;
	}
	void drawFrame() {
		RenderGate* renderGate = renderGates[currentFrame % CONCURRENT_RENDER_FRAMES];
		renderGate->targetImageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(device, target.swapchain, UINT64_MAX, renderGate->imageAvailability, VK_NULL_HANDLE, &renderGate->targetImageIndex.value());
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			target = RenderTarget(*this);
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		for (size_t i = 0; i < renderGates.size(); i++) {
			if (i == currentFrame % CONCURRENT_RENDER_FRAMES) {
				if (renderGates[i]->targetImageIndex.has_value() && renderGates[i]->targetImageIndex.value() == renderGate->targetImageIndex.value()) {
					vkWaitForFences(device, 1, &renderGates[i]->occupation, VK_TRUE, UINT64_MAX);
				}
			}
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore imageAvailabilityArray[] = { renderGate->imageAvailability };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = imageAvailabilityArray;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[renderGate->targetImageIndex.value()];
		VkSemaphore renderCompletenessArray[] = { renderGate->renderCompleteness };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = renderCompletenessArray;

		vkResetFences(device, 1, &renderGate->occupation);
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderGate->occupation) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer to graphics queue");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderCompletenessArray;
		VkSwapchainKHR swapChains[] = { target.swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &renderGate->targetImageIndex.value();

		vkQueuePresentKHR(presentQueue, &presentInfo);
		currentFrame++;
	}
	void destruct() {
		std::cout << "destructing App\n";
		for (auto& renderGate : renderGates) {
			renderGate->~RenderGate();
		}
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyPipelineLayout(device, layoutBundle.layout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, window.surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window.window);
		glfwTerminate();
	}
	std::vector<char> readFile(const std::string& filename) {
		// read file as binary, place cursor at end
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("failed to open file " + filename);
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0); // put cursor at start of file
		file.read(buffer.data(), fileSize); // read entire file, storing contents in buffer
		file.close();
		return buffer;
	}
};
