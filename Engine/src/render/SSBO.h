#pragma once

#include <cstdint>

class SSBO
{
public:
	SSBO();
	~SSBO();

	void bind();
	void setSlot(int index);
private:
	uint32_t id;
	int slot = 0;
};