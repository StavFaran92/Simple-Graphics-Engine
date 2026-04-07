#pragma once

#include "DialogBase.h"
#include <vector>

class ScriptSelectDialog : public DialogBase
{
public:
	ScriptSelectDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	int m_selectedScriptIndex = -1;
	std::vector<std::string> m_scriptNames;
};
