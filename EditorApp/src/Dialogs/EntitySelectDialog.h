#pragma once

#include "DialogBase.h"
#include "runtime/Entity.h"

class EntitySelectDialog : public DialogBase
{
public:
	EntitySelectDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	Entity m_selectedEntity = Entity::EmptyEntity;
};
