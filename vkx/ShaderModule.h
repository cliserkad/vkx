#pragma once
class ShaderModule;

class ShaderModule {
public:
	VkShaderModule shader;
	VkDevice device;

	~ShaderModule();
	VkPipelineShaderStageCreateInfo shaderCreateInfo(bool isFrag);
	ShaderModule(const std::vector<char>& code, const VkDevice& device);

};
