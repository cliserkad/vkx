#include "StandardIncludes.h"
#include "SwapChainSupport.cpp"
#include "ShaderModule.cpp""

// default window size of 720p
const uint32_t DEFAULT_WINDOW_WIDTH = 1280;
const uint32_t DEFAULT_WINDOW_HEIGHT = 720;

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
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class SingleTriangleApp {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		destruct();
	}
	~SingleTriangleApp() {
		destruct();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	QueueFamilyIndices indices;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkPipelineLayout pipelineLayout;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "SingleTriangleApp", nullptr, nullptr);
	}
	void initVulkan() {
		createInstance();
		createSurface();
		registerDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createPipeline();
	}
	void createPipeline() {
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");
		ShaderModule vertShaderModule = ShaderModule::create(vertShaderCode, device);
		ShaderModule fragShaderModule = ShaderModule::create(fragShaderCode, device);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule.shader;
		vertShaderStageInfo.pName = "main"; // set process name to main

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		fragShaderStageInfo.module = fragShaderModule.shader;
		fragShaderStageInfo.pName = "main"; // set process name to main

		VkPipelineShaderStageCreateInfo stages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) swapChainExtent.width;
		viewport.height = (float) swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // forces all vertices in frustrum to be within the acceptable depth range
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // wireframe, points, or regular
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // makes polygons one-sided
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// MSAA config
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// color blending. Affects how non-opaque colors are layered
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[]{
			VK_DYNAMIC_STATE_VIEWPORT
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 1;
		dynamicState.pDynamicStates = dynamicStates;

		// specify an empty pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkResult pipelineLayoutResult = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
		if (pipelineLayoutResult == VK_SUCCESS) {
			std::cout << "Instantiated a pipeline layout";
		} else {
			throw std::runtime_error("Failed to instantiate pipeline layout");
		}

		vertShaderModule.destruct();
		fragShaderModule.destruct();
	}
	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
	void createSwapChain() {
		SwapChainSupport swapChainSupport = SwapChainSupport::queryDevice(physicalDevice, surface);
		VkSwapchainCreateInfoKHR createInfo = swapChainSupport.buildInfoStruct(window, surface, indices);
		swapChainImageFormat = createInfo.imageFormat;
		swapChainExtent = createInfo.imageExtent;
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw new std::runtime_error("failed to create swapchain");
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data());
	}
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to link vulkan to glfw");
		}
	}
	void createLogicalDevice() {
		indices = QueueFamilyIndices::queryDevice(physicalDevice, surface);

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

		if (enableValidationLayers) {
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
		return QueueFamilyIndices::queryDevice(device, surface).isPopulated() && isDeviceExtended(device) && SwapChainSupport::queryDevice(device, surface).isAdequate();
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
		appInfo.pApplicationName = "SingleTriangleApp";
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
		if (enableValidationLayers) {
			creationInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			creationInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			creationInfo.enabledLayerCount = 0;
		}
		creationInfo.enabledLayerCount = 0; // no validation layers
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
		if(enableValidationLayers) {
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
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
	void destruct() {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
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

int main() {
	SingleTriangleApp app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
