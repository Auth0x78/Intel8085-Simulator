#pragma once
#include <iostream>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <bitset>

#include "memory.h"
#include "Logger.h"


/*
* Rules for flag affection are:
* 
* Sign Flag (S):
*	It is set if the result of the addition is negative (i.e., if the most significant bit of the result is 1).
*	Cleared if the result is positive (i.e., if the most significant bit is 0).
*
* Zero Flag (Z):
*	It is set if the result of the addition is zero.
*	Cleared if the result is non-zero.
*
* Auxiliary Carry Flag (AC):
*	It is set if there is a carry out from the lower nibble (i.e., from bit 3 to bit 4) during the addition.
*	Cleared if there is no carry from bit 3 to bit 4.
* 
* Parity Flag (P):
*	It is set if the result has an even number of 1s (even parity).
*	Cleared if the result has an odd number of 1s (odd parity).
* 
* Carry Flag (CY):
*	It is set if there is a carry out from the most significant bit (i.e., from bit 7) during the addition.
*	Cleared if there is no carry out from bit 7.
*/

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
	union
	{
		struct {
			uint8_t C;
			uint8_t B;
		};
		uint16_t BC;
	};
	union
	{
		struct
		{
			uint8_t E;
			uint8_t D;
		};
		uint16_t DE;
	};
	union
	{
		struct 
		{
			uint8_t L;
			uint8_t H;
		};
		uint16_t HL;
	};
	uint8_t FLAG; /*only lower 5 bits are used, rest upper 3 bits are not used */
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

	//Flag related functions
	inline bool			CheckFlag			(FlagType ftype);
	uint8_t&			GetReferenceToFlags	();
	void				SetAllFlags			(uint8_t word);
	void				SetSingleFlag		(FlagType ftype, bool _set);

	//Opcode Functions
	void			MOV						();
	void			ADC						(uint8_t& reg);
	void			ADD						(uint8_t& reg);
	void			ACI						();
	void			ADI						();
	void			LXI						(uint16_t& regPair);
	void			STAX					(uint16_t& regPair);
	void			INX						(uint16_t& regPair);
	void			INR						(uint8_t& reg);
	void			DCR						(uint8_t& reg);
	void			MVI						(uint8_t& reg);
	void			CMP						(uint8_t& reg);
	void			PUSH					(uint8_t& highRegister, uint8_t& lowRegister);
	void			POP						(uint8_t& highRegister, uint8_t& lowRegister);
	void			SUB						(uint8_t& reg);
	void			DCX						(uint16_t& regPair);
	void			LDAX					(uint16_t& regPair);
	void			JMP_CONDITIONAL			(bool toJump);
	

	//MISC.
	bool			Parity					(uint8_t reg);
	void			UpdateArithFlags		(uint16_t reg, bool updateCarry);


	//CPU STATE Variables
	_register m_regc;
	uint8_t m_currentInstr;
	bool m_isHlt;
	memory m_mem;
	
	//Class Variables
	std::vector<std::function<void()>> m_opcodeMap;
};

