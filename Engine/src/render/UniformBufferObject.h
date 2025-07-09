#pragma once
class UniformBufferObject
{
public:
	UniformBufferObject(int memsize);
	~UniformBufferObject();

	void attachToBindPoint(int bindingPointIndex);
	void setData(int offset, int size, const void* data);
	void bind() const;
	void unbind() const;
private:
	unsigned int m_id = 0;
};

