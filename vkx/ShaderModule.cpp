#include "ShaderModule.h"

#include <stdexcept>

class ShaderModule {
	public:
		VkShaderModule shader;
		VkDevice device;

		~ShaderModule() {
			vkDestroyShaderModule(device, shader, nullptr);
		}

		VkPipelineShaderStageCreateInfo shaderCreateInfo(bool isFrag) {
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			if (isFrag)
				shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			else
				shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStageInfo.module = shader;
			shaderStageInfo.pName = "main"; // set process name to main
			return shaderStageInfo;
		}

		ShaderModule(const std::vector<char>& code, const VkDevice& device) {
			this->device = device;
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS)
				throw std::runtime_error("Failed to instantiate ShaderModule");
		}

};
