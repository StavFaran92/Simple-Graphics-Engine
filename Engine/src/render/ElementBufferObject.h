#pragma once

#include <iostream>

class ElementBufferObject
{
public:
	ElementBufferObject(unsigned int* data, unsigned int length);
	~ElementBufferObject();

	void Bind() const;
	void Unbind() const;

	unsigned int getLength() const;
private:
	unsigned int m_id = 0;
	unsigned int m_length = 0;
};
