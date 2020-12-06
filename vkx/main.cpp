#include "StandardIncludes.h"
#include "QueueFamilyIndices.cpp"
#include "SwapChainSupport.cpp"

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
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;

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
	}
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to link vulkan to glfw");
		}
	}
	void createLogicalDevice() {
		QueueFamilyIndices indices = QueueFamilyIndices::queryDevice(physicalDevice, surface);

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
	void cleanup() {
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
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
