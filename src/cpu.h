#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <bitset>

#include "memory.h"
#include "Logger.h"


/*
* Instructions and data are in little endian format
* Register in a 8085 were:
*	Register	  Size		SSS/DDD
*	A				8		111
*	B				8		000
*	C				8		001
*	D				8		010
*	E				8		011
*	H				8		100
*	L				8		101
*	
*  For accessing memory, DDD/SSS is 110
*/

/*
*	'-' => No significance 
*	PSW in intel 8085 was:	
*		D7	D6	D5	D4	D3	D2	D1	D0 
*		S   Z	-	AC	-	P	-	CY
*/
enum FlagType
{
	//Flag Type			//MASK
	CarryFlag		=	0b00000001,
	ParityFlag		=	0b00000100,
	AuxCarryFlag	=	0b00010000,
	ZeroFlag		=	0b01000000,
	SignFlag		=	0b10000000,
};

struct _register
{
	uint8_t A;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t FLAG; /*only lower 5 bits are used, rest upper 3 bits are */
	uint16_t SP;
	uint16_t PC;
};

class CPU
{
public:
	//Constructors
	CPU					();
	CPU					(uint8_t* instructions, size_t SizeInByte);
	
	//Public Functions
	void				load_bytecode		(uint8_t* instructions, size_t SizeInByte);
	void				execute_single		(uint8_t instruction);
	void				step_one			();
	void				run					();
	void				snapshot			();

private:
	//Private Functions
	void				Init				();
	void				Not_Implemented		();
	inline uint8_t		fetch				();
	
	//Gets pointer, pointing to r register
	uint8_t*			GetPointerToRegister			(char reg);

	//Flag related functions
	inline bool			CheckFlag			(FlagType ftype);
	uint8_t&			GetReferenceToFlags	();
	void				SetAllFlags			(uint8_t word);
	void				SetSingleFlag				(FlagType ftype, bool _set);

	//Opcode Functions
	void			MOV						();
	void			LXI						(char reg);
	void			STAX					(char reg);
	void			INX						(char reg);
	void			INR						(char reg);
	void			MVI						(char reg);
	void			CMP						(char reg);
	void			PUSH					(char regX);
	void			POP						(char regX);
	void			JMP_CONDITIONAL			(bool toJump);
	void			LDAX					(char regX);

	//MISC.
	bool			Parity					(uint8_t reg);


	//CPU STATE Variables
	_register m_regc;
	uint8_t m_currentInstr;
	bool m_isHlt;
	memory m_mem;
	
	//Class Variables
	std::vector<std::function<void()>> m_opcodeMap;
};

