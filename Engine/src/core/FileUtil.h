#pragma once

//STL
#include <string>

#include "Engine.h"
#include "Window.h"
#include "commdlg.h"

class FileUtil
{
public:
    static std::string OpenFile(const char* filter);
    static std::string SaveFile(const char* filter);
};

