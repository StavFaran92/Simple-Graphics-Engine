#pragma once

#include "sge.h"

class GUIHandler : public GuiMenu
{
public:
    GUIHandler(Resource<Shader> shader);
    void display() override;


private:
    Resource<Shader> m_shader = nullptr;
};



