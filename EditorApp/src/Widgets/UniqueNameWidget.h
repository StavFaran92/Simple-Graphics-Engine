#pragma once

#include <string>

class UniqueNameWidget
{
public:
	UniqueNameWidget(const std::string& label);
	bool isValid() const;
	void draw();
	void clear();

	std::string m_label;
	std::string name;
};