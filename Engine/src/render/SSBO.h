#pragma once

#include <cstdint>

class SSBO
{
public:
	SSBO();
	~SSBO();

	void setData(int size, const void* data);
	void bind();
	void setSlot(int index);
private:
	uint32_t id;
	int slot = 0;
};