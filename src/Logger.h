#pragma once
#include <iostream>
#include <string>

// Define color escape sequences
#define RESET_COLOR		"\033[0m"
#define RED_COLOR		"\033[31m"
#define GREEN_COLOR		"\033[32m"
#define YELLOW_COLOR	"\033[33m"
#define BLUE_COLOR		"\033[34m"


enum LogLevel 
{
	Info = 0, Warning = 1, Error = 2, None = 3
};

class Logger
{
public:
	static	void		Log				(std::string&& message); /* Assume the message to level: none */
	static	void		Log				(LogLevel level, std::string&& message);
	static	void		SetLogLevel		(LogLevel level);
	static	LogLevel	GetLogLevel		();
private:
						Logger			();
	static	void		LogInfo			(std::string&& message);
	static	void		LogWarning		(std::string&& message);
	static	void		LogError		(std::string&& message);
private:
	static LogLevel _level;
};

