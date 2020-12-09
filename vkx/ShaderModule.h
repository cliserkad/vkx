#ifndef ShaderModule_h
#define ShaderModule_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class ShaderModule {
public:
	VkShaderModule shader;
	VkDevice device;

	~ShaderModule();
	VkPipelineShaderStageCreateInfo shaderCreateInfo(bool isFrag);
	ShaderModule(const std::vector<char>& code, const VkDevice& device);

};

#endif
