#include "QueueFamilyIndices.h"
#include "StandardImports.h"

class QueueFamilyIndices {
	private:
		bool isPopulated;
		uint32_t graphicsFamily;
	public:
		bool setGraphicsFamily(uint32_t graphicsFamily) {
			if (!isPopulated) {
				this->graphicsFamily = graphicsFamily;
				isPopulated = true;
				return true;
			} else
				return false;
		}
		uint32_t getGraphicsFamily() {
			return graphicsFamily;
		}
		bool isPopulated() {
			return isPopulated;
		}

};