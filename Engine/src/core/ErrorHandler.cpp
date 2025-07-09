#include "ErrorHandler.h"

#include "Logger.h"

int ErrorHandler::handle(const std::exception& e)
{
	logError("Error occured: " + std::string(e.what()));
	return 1;
}
