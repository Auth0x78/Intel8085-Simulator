#include "cpu.h"

#define REG_A	m_regc.A
#define REG_B	m_regc.B
#define REG_C	m_regc.C
#define REG_D	m_regc.D
#define REG_E	m_regc.E
#define REG_H	m_regc.H
#define REG_L	m_regc.L
#define	PTR_M	(((uint16_t)m_regc.H << 8) | m_regc.L)
#define REGX_BC  m_regc.BC
#define REGX_DE  m_regc.DE
#define REGX_HL  m_regc.HL


void CPU::Init()
{
	//Set is halt = false
	m_isHlt = false;

	//Set current instruction 0
	m_currentInstr = 0;
	
	//Register Setup
	m_regc.A = 0;
	m_regc.B = 0;
	m_regc.C = 0;
	m_regc.D = 0;
	m_regc.E = 0;
	m_regc.H = 0;
	m_regc.L = 0;
	m_regc.FLAG = 0;
	//As we have a 8KB memory we take the last 1KB of memory as stack;
	//Allocate 1KB Stack for a program
	//As stack grows from larger memory address to lower memory address
	m_regc.SP = (MEMORY_IN_BYTE - 1);
	m_regc.PC = 0;
	
	//Reserve m_opcodeMap with 256 and map them all to Not_Implemented first
	m_opcodeMap.reserve(256);
	for (int i = 0; i < 256; ++i) 
		m_opcodeMap.push_back([=]() { Not_Implemented(); });

	//NOP
	m_opcodeMap[0x0] = []() { (void)0; };

	//LXI OP
	m_opcodeMap[0x01] = [=]() { LXI(REGX_BC); }; 
	m_opcodeMap[0x11] = [=]() { LXI(REGX_DE); };
	m_opcodeMap[0x21] = [=]() { LXI(REGX_HL); };
	
	//LXI SP
	m_opcodeMap[0x31] = [=]() {
		m_regc.SP = fetch();
		m_regc.SP = ((uint16_t)fetch() << 8) | m_regc.SP;
	};

	//STAX OP
	m_opcodeMap[0x02] = [=]() { STAX(REGX_BC); }; //
	m_opcodeMap[0x12] = [=]() { STAX(REGX_DE); }; // +16

	//INX OP
	m_opcodeMap[0x03] = [=]() { INX(REGX_BC); };
	m_opcodeMap[0x13] = [=]() { INX(REGX_DE); };
	m_opcodeMap[0x23] = [=]() { INX(REGX_HL); };
	m_opcodeMap[0x33] = [=]() { m_regc.SP += 1; };

	//DCX OP
	m_opcodeMap[0x0B] = [=]() { DCX(REGX_BC); };
	m_opcodeMap[0x0B] = [=]() { DCX(REGX_DE); };
	m_opcodeMap[0x0B] = [=]() { DCX(REGX_HL); };
	m_opcodeMap[0x0B] = [=]() { m_regc.SP -= 1; };

	//DCR OP
	m_opcodeMap[0x3D] = [=]() { DCR(REG_A); };
	m_opcodeMap[0x05] = [=]() { DCR(REG_B); };
	m_opcodeMap[0x0D] = [=]() { DCR(REG_C); };
	m_opcodeMap[0x15] = [=]() { DCR(REG_D); };
	m_opcodeMap[0x1D] = [=]() { DCR(REG_E); };
	m_opcodeMap[0x25] = [=]() { DCR(REG_H); };
	m_opcodeMap[0x2D] = [=]() { DCR(REG_L); };
	//DCR M
	m_opcodeMap[0x35] = [=]() { DCR(m_mem[PTR_M]); };

	//INR OP
	m_opcodeMap[0x3C] = [=]() { INR(REG_A); };
	m_opcodeMap[0x04] = [=]() { INR(REG_B); };		
	m_opcodeMap[0x0C] = [=]() { INR(REG_C); };
	m_opcodeMap[0x14] = [=]() { INR(REG_D); };
	m_opcodeMap[0x1C] = [=]() { INR(REG_E); };
	m_opcodeMap[0x24] = [=]() { INR(REG_H); };
	m_opcodeMap[0x2C] = [=]() { INR(REG_L); };
	//INR M
	m_opcodeMap[0x34] = [=]() { INR(m_mem[PTR_M]); };
	

	//MVI OP
	m_opcodeMap[0x3E] = [=]() { MVI(REG_A); };
	m_opcodeMap[0x06] = [=]() { MVI(REG_B); };
	m_opcodeMap[0x0E] = [=]() { MVI(REG_C); };
	m_opcodeMap[0x16] = [=]() { MVI(REG_D); };
	m_opcodeMap[0x1E] = [=]() { MVI(REG_E); };
	m_opcodeMap[0x26] = [=]() { MVI(REG_H); };
	m_opcodeMap[0x2E] = [=]() { MVI(REG_L); };
	m_opcodeMap[0x36] = [=]() { MVI(m_mem[PTR_M]); };

	//MOV OP are from 0x40 to 0x7F, except 0x76 which is HLT
	for (uint8_t i = 0x40; i <= 0x7F; ++i) 
	{
		if (i == 0x76)
			m_opcodeMap[0x76] = [=]() { m_isHlt = true; };
		else
			m_opcodeMap[i] = [=]() { MOV(); };
	}
	
	// ADC OP
	m_opcodeMap[0x88] = [=]() {	ADC(REG_B); };
	m_opcodeMap[0x89] = [=]() {	ADC(REG_C); };
	m_opcodeMap[0x8A] = [=]() {	ADC(REG_D); };
	m_opcodeMap[0x8B] = [=]() {	ADC(REG_E); };
	m_opcodeMap[0x8C] = [=]() {	ADC(REG_H); };
	m_opcodeMap[0x8D] = [=]() {	ADC(REG_L); };
	m_opcodeMap[0x8F] = [=]() {	ADC(REG_A); };
	
	// ADC M
	m_opcodeMap[0x8E] = [=]() {	ADC(m_mem[PTR_M]); };

	//CMP OPERATION
	m_opcodeMap[0xBF] = [=]() { CMP(REG_A); };
	m_opcodeMap[0xB8] = [=]() { CMP(REG_B); };
	m_opcodeMap[0xB9] = [=]() { CMP(REG_C); };
	m_opcodeMap[0xBA] = [=]() { CMP(REG_D); };
	m_opcodeMap[0xBB] = [=]() { CMP(REG_E); };
	m_opcodeMap[0xBC] = [=]() { CMP(REG_H); };
	m_opcodeMap[0xBD] = [=]() { CMP(REG_L); };
	m_opcodeMap[0xBE] = [=]() { CMP(m_mem[PTR_M]); };
	
	//POP OPERATION
	m_opcodeMap[0xC1] = [=]() { POP(REG_B, REG_C); };
	m_opcodeMap[0xD1] = [=]() { POP(REG_D, REG_E); };
	m_opcodeMap[0xE1] = [=]() { POP(REG_H, REG_L); };
	//POP PSW
	m_opcodeMap[0xF1] = [=]() { POP(REG_A, m_regc.FLAG); };

	//PUSH OPERATION
	m_opcodeMap[0xC5] = [=]() { PUSH(REG_B, REG_C); };
	m_opcodeMap[0xD5] = [=]() { PUSH(REG_D, REG_E); };
	m_opcodeMap[0xE5] = [=]() { PUSH(REG_H, REG_L); };
	//PUSH PSW
	m_opcodeMap[0xF5] = [=]() { PUSH(REG_A, m_regc.FLAG); };

	/*PCHL (JUMP which copies HL itself into PC)*/
	m_opcodeMap[0xE9] = [=]() { m_regc.PC = PTR_M; };
	
	//JUMP OPERATIONs
	m_opcodeMap[0xC2] = [=]() { JMP_CONDITIONAL(!CheckFlag(ZeroFlag)); };
	m_opcodeMap[0xC3] = [=]() { JMP_CONDITIONAL(true); };
	m_opcodeMap[0xCA] = [=]() { JMP_CONDITIONAL(CheckFlag(ZeroFlag)); };
	
	m_opcodeMap[0xD2] = [=]() { JMP_CONDITIONAL(!CheckFlag(CarryFlag)); };
	m_opcodeMap[0xDA] = [=]() { JMP_CONDITIONAL(CheckFlag(CarryFlag)); };
	
	m_opcodeMap[0xE2] = [=]() { JMP_CONDITIONAL(!CheckFlag(ParityFlag)); };
	m_opcodeMap[0xEA] = [=]() { JMP_CONDITIONAL(CheckFlag(ParityFlag)); };
	
	m_opcodeMap[0xF2] = [=]() { JMP_CONDITIONAL(!CheckFlag(SignFlag)); };
	m_opcodeMap[0xFA] = [=]() { JMP_CONDITIONAL(CheckFlag(SignFlag)); };
	

	/*LDA Address*/
	m_opcodeMap[0x3A] = [=]() 
	{ 
		uint16_t mem_loc = fetch(); 
		mem_loc = ((uint16_t)fetch() << 8) | mem_loc;
		REG_A = m_mem[mem_loc];
	};
	//LDAX OPERATIOn
	m_opcodeMap[0x0A] = [=]() { LDAX(REGX_BC); };
	m_opcodeMap[0x1A] = [=]() { LDAX(REGX_DE); };
	
	/*LHLD Address*/
	m_opcodeMap[0x2A] = [=]() 
	{ 
		uint16_t mem_loc = fetch();
		mem_loc = ((uint16_t)fetch() << 8) | mem_loc;
		REG_L = m_mem[mem_loc++];
		REG_H = m_mem[mem_loc];
	};

}

CPU::CPU()
{
	Init();
}

CPU::CPU(uint8_t* instructions, size_t SizeInByte)
{
	// -1024 because the last 1KB is for the stack
	if (SizeInByte > (MEMORY_IN_BYTE - STACK_SIZE_IN_BYTE))
	{
		Logger::Log(Error, "Program occupying more memory than available!");
		exit(EXIT_FAILURE);
	}
	//Copy the contents of instructions into memory starting from 0
	memcpy_s(m_mem._ram, MEMORY_IN_BYTE, instructions, SizeInByte);

	Init();
}

void CPU::load_bytecode(uint8_t* instructions, size_t SizeInByte)
{
	// -1024 because the last 1KB is for the stack
	if (SizeInByte > (MEMORY_IN_BYTE - STACK_SIZE_IN_BYTE))
	{
		Logger::Log(Error, "Program occupying more memory than available!");
		exit(EXIT_FAILURE);
	}
	//Copy the contents of instructions into memory starting from 0
	memcpy_s(m_mem._ram, MEMORY_IN_BYTE, instructions, SizeInByte);
}

void CPU::execute_single(uint8_t instruction)
{
	//TODO: 
}

void CPU::step_one()
{
	//TODO: 
}

void CPU::run()
{
	//Fetch instruction at PC
	//Decode
	//Execute
	Logger::Log("CPU BEFORE: \n");
	snapshot();

	while(!m_isHlt)
	{
		m_currentInstr = fetch();
		m_opcodeMap[m_currentInstr]();
	}
	Logger::Log("CPU AFTER: \n");
	snapshot();
}

void CPU::snapshot()
{
	LogLevel prev = Logger::GetLogLevel();
	Logger::SetLogLevel(Info);

	Logger::Log(Info, "CPU Snapshot: ");
	std::cout << GREEN_COLOR 
		<< "\tA: " << static_cast<uint32_t> (m_regc.A) << "\n" 
		<< "\tB: " << static_cast<uint32_t> (m_regc.B) << "\n"
		<< "\tC: " << static_cast<uint32_t> (m_regc.C) << "\n"
		<< "\tD: " << static_cast<uint32_t> (m_regc.D) << "\n"
		<< "\tE: " << static_cast<uint32_t> (m_regc.E) << "\n"
		<< "\tH: " << static_cast<uint32_t> (m_regc.H) << "\n"
		<< "\tL: " << static_cast<uint32_t> (m_regc.L) << "\n"
		<< "\tFLAGS: " << std::bitset<8>(m_regc.FLAG) << "\n"
		<< "\tPC: " << m_regc.PC << "\n"
		<< "\tSP: " << m_regc.SP <<"\n"
		<< RESET_COLOR << std::endl;
	Logger::SetLogLevel(prev);
}

inline uint8_t CPU::fetch()
{
	return m_mem[m_regc.PC++];
}


void CPU::MOV()
{
	//Mask for the source is 0b00000111
	uint8_t Src = (m_currentInstr & 0b00000111);
	
	//Identify Source location
	switch (Src)
	{
		case 0x0:
			/*B*/
			Src = REG_B;
			break;
		case 0x1:
			Src = REG_C;
			break;
		case 0x2:
			Src = REG_D;
			break;
		case 0x3:
			Src = REG_E;
			break;
		case 0x4:
			Src = REG_H;
			break;
		case 0x5:
			Src = REG_L;
			break;
		case 0x6:
			/*Memory*/
			Src = m_mem[PTR_M];
			break;
		case 0x7:
			Src = REG_A;
			break;
	}

	//Identity Destination Location
	switch (((m_currentInstr & 0b00111000) >> 3))
	{
		case 0x0:
			/*B*/
			REG_B = Src;
			break;
		case 0x1:
			REG_C = Src;
			break;
		case 0x2:
			REG_D = Src;
			break;
		case 0x3:
			REG_E = Src;
			break;
		case 0x4:
			REG_H = Src;
			break;
		case 0x5:
			REG_L = Src;
			break;
		case 0x6:
			/*Memory*/
			m_mem[PTR_M] = Src;
			break;
		case 0x7:
			REG_A = Src;
			break;
	}
}
void CPU::ADC(uint8_t& reg)
{
	uint16_t carry = CheckFlag(FlagType::CarryFlag) ? 1 : 0;
	uint16_t result = REG_A + reg + carry;

	SetSingleFlag(FlagType::SignFlag, REG_A & 0b10000000 != 0);
	SetSingleFlag(FlagType::ZeroFlag, REG_A == 0);
	SetSingleFlag(FlagType::AuxCarryFlag, ((REG_A & 0x0F) + (reg & 0x0F) + carry) > 0x0F);
	SetSingleFlag(FlagType::ParityFlag, Parity(REG_A));
	SetSingleFlag(FlagType::CarryFlag, result > 0xFF);

	REG_A = result & 0xFF;
}
void CPU::LXI(uint16_t& regPair)
{
	/*	No flags are affected
	*	(BC, DE, HL) => {HIGH, LOW}
	*	1st fetch => Low byte
	*	2nd fetch => High byte
	*/
	regPair = fetch();
	regPair = (((uint16_t)fetch()) << 8) | regPair;
}
void CPU::STAX(uint16_t& regPair)
{
	/*	No flags are affected
	*	(BC, DE, HL) => {HIGH, LOW}
	*/
	m_mem[regPair] = REG_A;
}
void CPU::INX(uint16_t& regPair)
{
	/*
	* Increments register pair
	* Does not affect any flags
	* X Indicates, register pair;
	* (BC, DE, HL) => (HIGH-LOW)
	*/
	regPair += 1;
}
void CPU::INR(uint8_t& reg)
{
	/*
	* Increments register's content
	* Z: if result is 0
	* S: if result's msb is 1
	* P: if even bits in result
	*/
	reg += 1;

	//Set all the necessary flags accordinly
	SetSingleFlag(ZeroFlag, 0 == reg);
	SetSingleFlag(SignFlag, (0x80 == (reg & 0x80)));
	SetSingleFlag(ParityFlag, Parity(reg));
	SetSingleFlag(AuxCarryFlag, (reg & 0x0F) == 0x00);
}
void CPU::DCX(uint16_t& regPair)
{
	/*
	* Decrementss register pair
	* Does not affect any flags
	* X Indicates, register pair;
	* (BC, DE, HL) => (HIGH-LOW)
	*/

	regPair -= 1;
}
void CPU::DCR(uint8_t& reg)
{
	reg -= 1;

	//Set all the necessary flags accordinly
	SetSingleFlag(ZeroFlag, 0 == reg);
	SetSingleFlag(SignFlag, (0x80 == (reg & 0x80)));
	SetSingleFlag(ParityFlag, Parity(reg));
	
	SetSingleFlag(AuxCarryFlag, ((reg & 0x0F) < 1));
}
void CPU::MVI(uint8_t& reg)
{
	reg = fetch();
}
void CPU::CMP(uint8_t& reg)
{
	//Compares reg with A
	//Sub Contents of R from A, or A - r
	/*
	* CY: If A < r
	* Z: If A == r
	* S: IF MSB is 1
	* P: If even bits in result
	* Aux: Borrow from bit 3 to bit 4 during sub
	*/
	uint8_t sub_result = REG_A - reg;
	
	SetSingleFlag(ZeroFlag, 0 == sub_result);
	SetSingleFlag(SignFlag, (0x80 == (sub_result & 0x80)) );
	SetSingleFlag(ParityFlag, Parity(sub_result));
	SetSingleFlag(AuxCarryFlag, ((REG_A & 0x0F) < (reg & 0x0F)) );
	SetSingleFlag(CarryFlag, REG_A < reg);
}
void CPU::PUSH(uint8_t& highRegister, uint8_t& lowRegister)
{
	/*
	* X => eXtended
	* NOTE:		| SP is pre - decrement, because SP points to valid data
	* First Push the high byte on stack,
	* then dec stack ptr,
	* then push low byte
	* Implement this for PUSH PC
	*/

	m_mem[--m_regc.SP] = highRegister; // PUSH HIGHER byte
	m_mem[--m_regc.SP] = lowRegister; //PUSH LOWER byte
}
void CPU::POP(uint8_t& highRegister, uint8_t& lowRegister)
{
	/*
	* POPs the top of the stack into register pairs
	* Low byte appears first, then High order byte
	* Stack already points to a valid byte, so we
	* post-increment to get the next byte
	* Implement this for POP PC
	*/
	lowRegister = m_mem[m_regc.SP++];
	highRegister = m_mem[m_regc.SP++];
}
void CPU::JMP_CONDITIONAL(bool toJump)
{
	/*
	* Sets PC to the absolute address given with JMP (conditional)
	*/
	if (!toJump)
		return;

	/*Fetch the lower byte of the jump address*/
	uint16_t jmpaddr = fetch();
	/*Fetch the high byte of the jump address and shift it up by 8*/
	jmpaddr = ((uint16_t)fetch() << 8) | jmpaddr;
	
	//SET PC to the jmp address
	m_regc.PC = jmpaddr;
}
void CPU::LDAX(uint16_t& regPair)
{
	REG_A = m_mem[regPair];
}

bool CPU::Parity(uint8_t num)
{
	int p = 0;
	for (int i = 0; i < 8; i++) {
		if (num & 0x1) {
			p++;
		}
		num = num >> 1;
	}
	return ((p & 0x1) == 0);
}
void CPU::Not_Implemented()
{
	Logger::Log(Error, "Opcode "+ std::to_string(m_currentInstr) + " not implemented!");
	exit(EXIT_FAILURE);
}

inline bool CPU::CheckFlag(FlagType ftype)
{
	return (m_regc.FLAG & (uint8_t)ftype) != 0;
}
uint8_t& CPU::GetReferenceToFlags()
{
	return m_regc.FLAG;
}
void CPU::SetAllFlags(uint8_t byte)
{
	m_regc.FLAG = byte;
}
void CPU::SetSingleFlag(FlagType ftype, bool _set)
{
	if (_set)
		m_regc.FLAG = m_regc.FLAG | (static_cast<uint8_t>(ftype));
	else
		m_regc.FLAG = m_regc.FLAG & (~static_cast<uint8_t>(ftype));
}
