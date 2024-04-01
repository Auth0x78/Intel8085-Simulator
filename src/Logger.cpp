#include "logger.h"

//Set default value of _level
LogLevel Logger::_level = LogLevel::Info;

//Constructor
Logger::Logger() {}

//Public Functions 
void Logger::Log(std::string&& message) /*Assume the log level to be none*/
{
	std::cout << GREEN_COLOR << "[] " << RESET_COLOR << message;
}

void Logger::Log(LogLevel level, std::string&& message)
{
	if (level < _level)
		return;
	switch (level)
	{
	case Info:
		LogInfo(std::move(message));
		break;
	case Warning:
		LogWarning(std::move(message));
		break;
	case Error:
		LogError(std::move(message));
		break;
	case None:
		std::cout << RESET_COLOR << message;
		break;
	default:
		break;
	}
}

void Logger::SetLogLevel(LogLevel level) 
{ 
	_level = level; 
}

LogLevel Logger::GetLogLevel() 
{ 
	return _level; 
}

//Private Functions
void Logger::LogInfo(std::string&& message)
{
	std::cout << BLUE_COLOR << "[INFO]: " << RESET_COLOR << message << std::endl;
}

void Logger::LogWarning(std::string&& message)
{
	std::cout << YELLOW_COLOR << "[WARN]: " << RESET_COLOR << message << std::endl;
}

void Logger::LogError(std::string&& message)
{
	std::cout << RED_COLOR << "[ERROR]: " << RESET_COLOR << message << std::endl;
}
