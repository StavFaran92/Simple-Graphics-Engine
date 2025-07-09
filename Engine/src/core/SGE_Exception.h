#pragma once

#include <exception>
#include <string>

class SGE_Exception : public std::runtime_error
{
public:
	SGE_Exception(char const* className, char const* funcName, int lineID, char const* msg)
		: std::runtime_error(msg)
	{
		eMsg = "Filename :" + std::string(className) + 
			", Function: " + std::string(funcName) + 
			", Line: " + std::to_string(lineID) + 
			", Msg: " + std::string(msg);
	}
	char const* what() const override
	{
		return eMsg.c_str();
	}

private:
	std::string eMsg;
};

#define SGE_THROW(msg) throw SGE_Exception(__FILE__, __FUNCTION__, __LINE__, msg)