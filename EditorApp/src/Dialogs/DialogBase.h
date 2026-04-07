#pragma once

#include <string>

class DialogBase
{
public:
	DialogBase(const std::string& label);
	virtual ~DialogBase() = default;
	void draw();
	void header();
	void footer();

	void activate();
	void display();

protected:
	virtual void appearContent() = 0;
	virtual void drawContent() = 0;
	virtual bool acceptContent() = 0;
	virtual void cancelContent() = 0;
	virtual void headerContent() {};
	virtual void footerContent() {};

	const std::string m_label;
};