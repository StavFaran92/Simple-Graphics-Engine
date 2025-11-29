#pragma once

class GUILayer
{
public:
	virtual bool init() = 0;
	virtual void render() = 0;
	virtual void close() = 0;
};

