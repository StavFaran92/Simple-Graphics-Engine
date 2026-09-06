#pragma once

#include <cstdint>

class SSBO
{
public:
	SSBO();
	~SSBO();

	void allocate(int size);
	void setData(int size, const void* data);

	// Appends data at the current write cursor and advances it, returning the byte offset it was written at.
	// Call resetCursor() at the start of a frame/pass before appending.
	int pushData(int size, const void* data);
	void resetCursor();

	void bind();
	void setSlot(int index);
private:
	uint32_t id;
	int slot = 0;
	int cursor = 0;
};