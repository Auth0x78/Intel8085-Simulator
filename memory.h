#pragma once
#include <iostream>

#include "Logger.h"

#define MEMORY_IN_BYTE 8 * 1024
#define STACK_SIZE_IN_BYTE 1 * 1024

class memory
{
public:
	memory();

	//Operator overload
	uint8_t& operator[] (uint16_t location);
	uint8_t _ram[MEMORY_IN_BYTE];
};

