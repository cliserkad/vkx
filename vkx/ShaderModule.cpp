#include "StandardIncludes.h"

class ShaderModule {
	public:
		VkShaderModule shader;
		VkDevice device;

		~ShaderModule() {
			destruct();
		}

		void destruct() {
			vkDestroyShaderModule(device, shader, nullptr);
		}

		static ShaderModule create(const std::vector<char>& code, const VkDevice& device) {
			ShaderModule shaderModule;
			shaderModule.device = device;
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
			VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule.shader);
			if (result == VK_SUCCESS)
				std::cout << "Instantiated a ShaderModule";
			else
				throw std::runtime_error("Failed to instantiate ShaderModule");
			return shaderModule;
		}


};