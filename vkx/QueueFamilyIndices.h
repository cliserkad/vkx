#pragma once
class QueueFamilyIndices {
public:
	bool isPopulated();
	bool setGraphicsFamily(uint32_t graphicsFamily);
	uint32_t getGraphicsFamily();
};

