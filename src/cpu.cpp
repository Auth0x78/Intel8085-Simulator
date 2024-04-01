#include "cpu.h"

#define REG_A	m_regc.A
#define REG_B	m_regc.B
#define REG_C	m_regc.C
#define REG_D	m_regc.D
#define REG_E	m_regc.E
#define REG_H	m_regc.H
#define REG_L	m_regc.L
#define	PTR_M	(((uint16_t)m_regc.H << 8) | m_regc.L)


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

	//Register Appearance order (not Pairs)
	std::vector<char> regApp =
	{
		'I',
		'B', //1
		'C', //2
		'D', //3
		'E', //4
		'H', //5
		'L', //6
		'M', //7
		'A', //8
	};


	//NOP
	m_opcodeMap[0x0] = []() { (void)0; };

	//LXI OP
	m_opcodeMap[0x01] = [=]() { LXI('B'); }; //  
	m_opcodeMap[0x11] = [=]() { LXI('D'); }; // +16
	m_opcodeMap[0x21] = [=]() { LXI('H'); }; // +16
	m_opcodeMap[0x31] = [=]() { LXI('S'); }; // +16

	//STAX OP
	m_opcodeMap[0x02] = [=]() { STAX('B'); }; //
	m_opcodeMap[0x12] = [=]() { STAX('D'); }; // +16

	//INX OP
	m_opcodeMap[0x03] = [=]() { INX('B'); };
	m_opcodeMap[0x13] = [=]() { INX('D'); };
	m_opcodeMap[0x23] = [=]() { INX('H'); };
	m_opcodeMap[0x33] = [=]() { INX('S'); };

	//INR OP (AP of a=4, d=8, nterms=8)
	//an = a + (n - 1) d;
	for(uint8_t n = 1; n <= 8; ++n)
		m_opcodeMap[4 + (n - 1) * 8] = [=]() { INR(regApp[n]); };
	//m_opcodeMap[0x04] = [=]() { INR('B'); };//4		
	//m_opcodeMap[0x0C] = [=]() { INR('C'); };//12	+8
	//m_opcodeMap[0x14] = [=]() { INR('D'); };//20	+8
	//m_opcodeMap[0x1C] = [=]() { INR('E'); };//28	+8
	//m_opcodeMap[0x24] = [=]() { INR('H'); };//36	+8
	//m_opcodeMap[0x2C] = [=]() { INR('L'); };//44	+8
	//m_opcodeMap[0x34] = [=]() { INR('M'); };//52	+8
	//m_opcodeMap[0x3C] = [=]() { INR('A'); };//60	+8
	

	//MVI OP (AP of a=6, d=8, nterms=8)
	//an = a + (n - 1) d;
	for (uint8_t n = 1; n <= 8; ++n)
		m_opcodeMap[6 + (n - 1) * 8] = [=]() { MVI(regApp[n]); };
	//m_opcodeMap[0x06] = [=]() { MVI('B'); };
	//m_opcodeMap[0x0E] = [=]() { MVI('C'); };
	//m_opcodeMap[0x16] = [=]() { MVI('D'); };
	//m_opcodeMap[0x1E] = [=]() { MVI('E'); };
	//m_opcodeMap[0x26] = [=]() { MVI('H'); };
	//m_opcodeMap[0x2E] = [=]() { MVI('L'); };
	//m_opcodeMap[0x36] = [=]() { MVI('M'); };
	//m_opcodeMap[0x3E] = [=]() { MVI('A'); };

	//MOV OPERATION
	for (uint8_t i = 0x40; i <= 0x7F; ++i) {
		if (i == 0x76)
			m_opcodeMap[0x76] = [=]() { m_isHlt = true; };
		else
			m_opcodeMap[i] = [=]() { MOV(); };
	}

	//CMP OPERATION AP of a = 184, d = 1 nterms = 8
	for (uint8_t n = 1; n <= 8; ++n)
		m_opcodeMap[184 - 1 + n] = [=]() { CMP(regApp[n]); };
	
	//POP OPERATION, AP of a = 193, d = 16, nterms = 3
	for (uint8_t n = 1; n <= 3; ++n)
		m_opcodeMap[193 + (n - 1) * 16] = [=]() { POP(regApp[2 * n - 1]); };
	m_opcodeMap[0xF1] = [=]() { POP('A'); };
  
	//PUSH OPERATION, AP of a = 197, d = 16, nterms = 3
	for (uint8_t n = 1; n <= 3; ++n)
		m_opcodeMap[197 + (n - 1) * 16] = [=]() { PUSH(regApp[2 * n - 1]); };
	m_opcodeMap[0xF5] = [=]() { PUSH('A'); };

	//TODO: JUMP OPERATIONs
	m_opcodeMap[0xC2] = [=]() { JMP_CONDITIONAL(!CheckFlag(ZeroFlag)); };
	m_opcodeMap[0xC3] = [=]() { JMP_CONDITIONAL(true); };
	m_opcodeMap[0xCA] = [=]() { JMP_CONDITIONAL(CheckFlag(ZeroFlag)); };
	
	m_opcodeMap[0xD2] = [=]() { JMP_CONDITIONAL(!CheckFlag(CarryFlag)); };
	m_opcodeMap[0xDA] = [=]() { JMP_CONDITIONAL(CheckFlag(CarryFlag)); };
	
	m_opcodeMap[0xE2] = [=]() { JMP_CONDITIONAL(!CheckFlag(ParityFlag)); };
	m_opcodeMap[0xEA] = [=]() { JMP_CONDITIONAL(CheckFlag(ParityFlag)); };
	
	m_opcodeMap[0xF2] = [=]() { JMP_CONDITIONAL(!CheckFlag(SignFlag)); };
	m_opcodeMap[0xFA] = [=]() { JMP_CONDITIONAL(CheckFlag(SignFlag)); };
	
	/*PCHL (JUMP which copies HL itself into PC)*/
	m_opcodeMap[0xE9] = [=]() { m_regc.PC = PTR_M; };

	/*LDA Address*/
	m_opcodeMap[0x3A] = [=]() 
	{ 
		uint16_t mem_loc = fetch(); 
		mem_loc = ((uint16_t)fetch() << 8) | mem_loc;
		REG_A = m_mem[mem_loc];
	};
	m_opcodeMap[0x0A] = [=]() { LDAX('B'); };
	m_opcodeMap[0x1A] = [=]() { LDAX('D'); };
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

uint8_t* CPU::GetPointerToRegister(char reg)
{
	switch (reg)
	{
		case 'A':
			return &m_regc.A;
		case 'B':
			return &m_regc.B;
		case 'C':
			return &m_regc.C;
		case 'D':
			return &m_regc.D;
		case 'E':
			return &m_regc.E;
		case 'H':
			return &m_regc.H;
		case 'L':
			return &m_regc.L;
		default:
			Logger::Log(Warning, "Unknow register: '" + std::string(&reg, 1) + "' !");
			exit(EXIT_FAILURE);
	}
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

void CPU::LXI(char reg)
{
	/*	No flags are affected
	*	(BC, DE, HL) => {HIGH, LOW}
	*/
	
	switch (reg) 
	{
		case 'B':
			REG_C = fetch();
			REG_B = fetch();
			break;
		case 'D':
			REG_E = fetch();
			REG_D = fetch();
			break;
		case 'H':
			REG_L = fetch();
			REG_H = fetch();
			break;
		case 'S':
			//Fetch lower 8 bits
			m_regc.SP = fetch();
			//Fetch higher 8 bits
			m_regc.SP = ((uint16_t)fetch() << 8) | m_regc.SP;
			break;
		default:
			Logger::Log(Warning, "LXI " + std::string(&reg, 1) + "is not valid!");
			exit(EXIT_FAILURE);
	}
	/*Load BC Register with immediate value*/
}

void CPU::STAX(char reg)
{
	/*	No flags are affected
	*	(BC, DE, HL) => {HIGH, LOW}
	*/
	switch (reg)
	{
		case 'B':
			m_mem[((uint16_t)REG_B << 8) | REG_C] = REG_A;
			break;
		case 'D':
			m_mem[((uint16_t)REG_D << 8) | REG_E] = REG_A;
			break;
		default:
			Logger::Log(Warning, "STAX " + std::string(&reg, 1) + "is not valid!");
			exit(EXIT_FAILURE);
	}
}

void CPU::INX(char reg)
{
	/*
	* Increments register pair
	* Does not affect any flags
	* X Indicates, register pair;
	* (BC, DE, HL) => (HIGH-LOW) 
	*/
	uint16_t temp = 0;
	switch (reg)
	{
		case 'B':
			temp = ((uint16_t)REG_B << 8 | REG_C);
			temp += 1;
			REG_B = (temp & 0xFF00) >> 8;
			REG_C = temp & 0x00FF;
			break;
		case 'D':
			temp = ((uint16_t)REG_D << 8 | REG_E);
			temp += 1;
			REG_D = (temp & 0xFF00) >> 8;
			REG_E = temp & 0x00FF;
			break;
		case 'H':
			temp = ((uint16_t)REG_H << 8 | REG_L);
			temp += 1;
			REG_H = (temp & 0xFF00) >> 8;
			REG_L = temp & 0x00FF;
			break;
		case 'S':
			/*Increment Stack Pointer*/
			m_regc.SP += 1;
			break;
		default:
			Logger::Log(Warning, "INX " + std::string(&reg, 1) + " is not valid!");
			exit(EXIT_FAILURE);
	}
}

void CPU::INR(char reg)
{
	/*
	* Increments register's content
	* Z: if result is 0
	* S: if result's msb is 1
	* P: if even bits in result
	*/
	//Get the pointer the wanted register and then increment its content
	uint8_t* target = nullptr;

	if (reg == 'M')
	{
		 target = &m_mem[PTR_M];
		*target += 1;
	}
	else
	{
		target = GetPointerToRegister(reg);
		*target += 1;
	}


	//Set all the necessary flags accordinly
	SetSingleFlag(ZeroFlag, 0 == *target);
	SetSingleFlag(SignFlag, (0x80 == (*target & 0x80)));
	SetSingleFlag(ParityFlag, Parity(*target));
	SetSingleFlag(AuxCarryFlag, (*target & 0x0F) == 0x00);
}

void CPU::MVI(char reg)
{
	if (reg == 'M')
		m_mem[PTR_M] = fetch();
	else
		*GetPointerToRegister(reg) = fetch();
}

void CPU::CMP(char reg)
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
	uint8_t sub_result = 0;
	auto setupFlags = [=](uint8_t* target)
		{
			SetSingleFlag(ZeroFlag, 0 == sub_result);
			SetSingleFlag(SignFlag, (0x80 == (sub_result & 0x80)) );
			SetSingleFlag(ParityFlag, Parity(sub_result));
			SetSingleFlag(AuxCarryFlag, ((REG_A & 0x0F) < ((*target) & 0x0F)) );
			SetSingleFlag(CarryFlag, REG_A < *target);	
		};

	if (reg == 'M')
	{
		sub_result = REG_A - m_mem[PTR_M];
		setupFlags(&m_mem[PTR_M]);
	}
	else
	{
		sub_result = REG_A - *GetPointerToRegister(reg);
		setupFlags(GetPointerToRegister(reg));
	}
}

void CPU::PUSH(char regX)
{
	/*
	* X => eXtended
	* NOTE:		| SP is pre - decrement, because SP points to valid data
	* First Push the high byte on stack,
	* then dec stack ptr,
	* then push low byte
	* Implement this for PUSH PC
	*/

	switch (regX)
	{
		case 'A':
			/*PUSH PSW (A_FLAGs)*/
			m_mem[--m_regc.SP] = *GetPointerToRegister('A');
			m_mem[--m_regc.SP] = GetReferenceToFlags();
			break;
		case 'B':
			m_mem[--m_regc.SP] = *GetPointerToRegister('B');
			m_mem[--m_regc.SP] = *GetPointerToRegister('C');
			break;
		case 'D':
			m_mem[--m_regc.SP] = *GetPointerToRegister('D');
			m_mem[--m_regc.SP] = *GetPointerToRegister('E');
			break;
		case 'H':
			m_mem[--m_regc.SP] = *GetPointerToRegister('H');
			m_mem[--m_regc.SP] = *GetPointerToRegister('L');
			break;
		case 'P':
			/*PUSH PC*/
			m_mem[--m_regc.SP] = static_cast<uint8_t>(m_regc.PC >> 8);
			m_mem[--m_regc.SP] = static_cast<uint8_t>(m_regc.PC & 0x00FF);
			break;
		default:
			Logger::Log(Error, "Unknow PUSH: " + std::string(&regX, 1));
			exit(EXIT_FAILURE);
	}
}

void CPU::POP(char regX)
{
	/*
	* POPs the top of the stack into register pairs
	* Low byte appears first, then High order byte
	* Stack already points to a valid byte, so we
	* post-increment to get the next byte
	* Implement this for POP PC
	*/
	switch (regX)
	{
		case 'A':
			/*POP PSW (A_FLAGs)*/
			SetAllFlags(m_mem[m_regc.SP++]);
			*GetPointerToRegister('A') = m_mem[m_regc.SP++];
			break;
		
		case 'B':
			*GetPointerToRegister('C') = m_mem[m_regc.SP++];
			*GetPointerToRegister('B') = m_mem[m_regc.SP++];
			break;
		
		case 'D':
			*GetPointerToRegister('E') = m_mem[m_regc.SP++];
			*GetPointerToRegister('D') = m_mem[m_regc.SP++];
			break;
		
		case 'H':
			*GetPointerToRegister('L') = m_mem[m_regc.SP++];
			*GetPointerToRegister('H') = m_mem[m_regc.SP++];
			break;
		case 'P':
			m_regc.PC = m_mem[m_regc.SP] | ((uint16_t)m_mem[m_regc.SP + 1] << 8);
			m_regc.SP += 2;
			break;
		default:
			Logger::Log(Error, "Unknow POP: " + std::string(&regX, 1));
			exit(EXIT_FAILURE);
	}
}

void CPU::JMP_CONDITIONAL(bool toJump)
{
	/*
	* 
	* Sets PC to the absolute address given with JMP (conditional)
	*/
	if (!toJump)
		return;
	/*Fetch the lower byte of the jump address*/
	uint16_t jmpaddr = fetch();
	/*Fetch the high byte of the jump address and shift it up by 8*/
	jmpaddr = ((uint16_t)fetch() << 8) | jmpaddr;
	
	m_regc.PC = jmpaddr;
}

void CPU::LDAX(char regX)
{
	switch (regX)
	{
		case 'B':
			REG_A = m_mem[((uint16_t)REG_B << 8) | REG_C];
			break;
		case 'D':
			REG_A = m_mem[((uint16_t)REG_D << 8) | REG_E];
			break;
		default:
			Logger::Log(Error, "LDAX " + std::string(&regX, 1) + " is not defined!");
			break;
	}
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
