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

int main() {
	Renderer app;

	try {
		app.mainLoop();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}