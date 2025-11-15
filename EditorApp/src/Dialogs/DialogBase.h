#pragma once

#include <string>

class DialogBase
{
public:
	virtual ~DialogBase() = default;
	void appear();
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

	const std::string m_label;
};