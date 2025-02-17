#include "memory.h"

memory::memory()
{
	for (auto& byte : _ram)
		byte = 0;
}

uint8_t& memory::operator[](uint16_t location)
{
	if (location < MEMORY_IN_BYTE && location >= 0)
		return _ram[location];
	else
	{
		Logger::Log(Error, "Memory location out of allocated bounds!");
		exit(EXIT_FAILURE);
	}
}
