#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

//My libraries
#include "Logger.h"
#include "cpu.h"

//8-bit intel 8085 simulator

int main() 
{
	//Variable initialisation
	CPU vcpu;

	{
		char* bytecode;
		std::string filepath;
		
		//Take file path from user
		Logger::Log("Enter file path: ");
		std::cin >> filepath;

		//Open file for reading
		std::ifstream infile(filepath, std::ios_base::in | std::ios_base::binary);
		
		if (!infile.is_open()) 
		{
			Logger::Log(Error, "Failed to open file!");
			return EXIT_FAILURE;
		}
		
		//Read file into buf
		std::streampos fsize = infile.tellg();
		infile.seekg(0, std::ios::end);
		fsize = infile.tellg() - fsize;
		//Reset the file cursor to start
		infile.seekg(0, std::ios::beg);

		size_t fileSize = static_cast<size_t>(fsize);
		bytecode = new char[fileSize]();
		infile.read(bytecode, fileSize);

		vcpu.load_bytecode(reinterpret_cast<uint8_t*>(bytecode), fileSize);
		infile.close();
	}

	vcpu.run();

	return EXIT_SUCCESS;
}