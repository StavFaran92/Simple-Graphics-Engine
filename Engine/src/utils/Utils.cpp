#include "Utils.h"
#include "Logger.h"

std::string Utils::ReadFile(const std::string& filePath)
{
	std::string content = "";
	std::ifstream fileStream;
	fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		fileStream.open(filePath.c_str());

		// read file's buffer contents into streams
		std::stringstream stream;
		stream << fileStream.rdbuf();

		// close file handlers
		fileStream.close();

		// convert stream into string
		content = stream.str();
	}
	catch (std::ifstream::failure e)
	{
		logError( "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" );

		throw std::runtime_error("File not found: " + filePath);
	}

	return content;
}