#pragma once

#include <string>

class FilepathWidget
{
public:
	FilepathWidget(const std::string& label, char const* const* formats, size_t formatCount);
	bool isValid() const;
	void draw();
	void clear();
	bool accept();

	std::string m_filepath;
	bool m_isPressed = false;
	std::string m_label;
	size_t m_formatCount = 0;
	const char* const* m_formats = nullptr;
};