#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "StandardImports.h"
#include "QueueFamilyIndices.h"

// default window size of 720p
const uint32_t DEFAULT_WINDOW_WIDTH = 1280;
const uint32_t DEFAULT_WINDOW_HEIGHT = 720;

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

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "SingleTriangleApp", nullptr, nullptr);
	}
	void initVulkan() {
		createInstance();
		registerDevice();
		createLogicalDevice();
	}
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.getGraphicsFamily();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		if (result == VK_SUCCESS) {
			std::cout << "created vulkan virtual device\n";
		} else {
			throw std::runtime_error("failed to create virtual vulkan device");
		}

		vkGetDeviceQueue(device, indices.getGraphicsFamily(), 0, &graphicsQueue);
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
		QueueFamilyIndices indices = findQueueFamilies(device);
		return indices.isPopulated();
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
			returnFalse:
			return false;
			nextLayer:
			continue;
		}
		return true;
	}
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.setGraphicsFamily(i);
			if (indices.isPopulated())
				break;
			i++;
		}
		return indices;
	}
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
	void cleanup() {
		vkDestroyDevice(device, nullptr);
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
