#include <iostream>
#include <fstream>
#include <string>
using namespace std;

enum mnem {
	UNDEFINED = 0, RRC, SWRB, RRA,
	SXT, PUSH, CALL, RETI,
	NOTUSED,

	JNE = 10, JEQ, JNC, JC,
	JN, JGE, JL, JMP, 	

	MOV = 20, ADD, ADDC, SUBC,
	SUB, CMP, DADD, BIT,
	BIC, BIS, XOR, AND,

	NOP = 35, POP, BR, RET,
	CLRC, SETC, CLRZ, SETZ,
	CLRN, SETN, DINT, EINT,
	RLA, RLC, INV, CLR,
	TST, DEC, DECD, INC,
	INCD, ADC, DADC, SBC,
	ERROR
};

string map[] = {
	"undefined", "rrc", "swrb", "rra",
	"sxt", "push", "call", "reti",
	"notused", "9"

	"jne", "jeq", "jnc", "jc",
	"jn", "jge", "jl", "jmp",
	"18", "19",

	"mov", "add", "addc", "subc",
	"sub", "cmp", "dadd", "bit",
	"bic", "bis", "xor", "and",
	"32","33","34"

	"nop", "pop", "br", "ret",
	"clrc", "setc", "clrz", "setz",
	"clrn", "setn", "dint", "eint",
	"rla", "rlc", "inv", "clr",
	"tst", "dec", "decd", "inc",
	"incd", "adc", "dadc", "sbc",
	"error"
};



enum registers {
	PC, SP, SR, CG,
	R4, R5, R6, R7,
	R8, R9, R10, R11,
	R12, R13, R14, R15
};

enum addressing_mode
{
	register_mode = 0, indexed_mode, symbolic_mode, absolute_mode,
	indirect_register_mode, indirect_autoincrement, immediate_mode,
	constantneg1, constant0, constant1, constant2, constant4,
	constant8, error
};

struct instruction
{
	struct operand
	{
		registers source_reg;
		registers dest_reg;
		uint16_t imm;
		mnem instruction;
	}op[3];
	bool bw;
	addressing_mode destination;
	addressing_mode source;
	bool error;
	int optype;
};

int msp430Decompose(uint16_t value[3], instruction &set)
{
	int amount_used = 0;
	union {
		uint16_t full;
		struct 
		{
			registers source_reg:4;
			uint16_t as:2;
			uint16_t bw:1;
			mnem opcode:3;
			uint16_t bitset:6;
		}op1;
		struct 
		{
			uint16_t pc_offset:10;
			mnem condition:3;
			uint16_t bitset:3;
		}op2;
		struct 
		{
			registers dest_reg:4;
			uint16_t as:2;
			uint16_t bw:1;
			uint16_t ad:1;
			registers source_reg:4;
			mnem opcode:4;
		}op3;
	}decode;
	decode.full = value[0];

	set.error = false;
	if (decode.op1.bw == 0)
	{
		set.bw = false;
	}
	else
	{
		set.bw = true;
	}

	if (decode.op3.opcode == 1)
	{
		set.optype = 1;
	}
	else if (decode.op3.opcode == 2 || decode.op3.opcode == 3)
	{
		set.optype = 2;
	}
	else if (decode.op3.opcode >=4)
	{
		set.optype = 3;
	}
	switch (set.optype)
	{
		case 0:
			set.error = true;
			break;
		case 1:
			set.op[0].instruction = decode.op1.opcode;
			set.op[0].source_reg = decode.op1.source_reg;
			switch(decode.op1.as)
			{
				case 0:
					if (decode.op1.source_reg != 3)
						set.source = register_mode;
					else if (decode.op1.source_reg == 3)
						set.source = constant0;
					break;
				case 1:
					if (decode.op1.source_reg == PC)
						set.source = symbolic_mode;
					else if (decode.op1.source_reg == SR)
						set.source = absolute_mode;
					else if (decode.op1.source_reg == CG)
						set.source = constant1;
					else
						set.source = indexed_mode;
					break;
				case 2:
					if (decode.op1.source_reg == SR)
						set.source = constant4;
					else if (decode.op1.source_reg == CG)
						set.source = constant2;
					else
						set.source = indirect_register_mode;
					break;
				case 3:
					if (decode.op1.source_reg == PC)
						set.source = immediate_mode;
					else if (decode.op1.source_reg == SR)
						set.source = constant8;
					else if (decode.op1.source_reg == CG)
						set.source = constantneg1;
					else
						set.source = indirect_autoincrement;
					break;
				default: break;
			}
			break;
		case 2:
			set.op[1].instruction = (mnem)(decode.op2.condition + JNE);
			set.op[1].imm = decode.full;
			break;
		case 3:
			switch(decode.op1.as)
			{
				case 0:
					if (decode.op3.source_reg != 3)
						set.source = register_mode;
					else if (decode.op3.source_reg == 3)
						set.source = constant0;
					break;
				case 1:
					if (decode.op3.source_reg == PC)
						set.source = symbolic_mode;
					else if (decode.op3.source_reg == SR)
						set.source = absolute_mode;
					else if (decode.op3.source_reg == CG)
						set.source = constant1;
					else
						set.source = indexed_mode;
					break;
				case 2:
					if (decode.op3.source_reg == SR)
						set.source = constant4;
					else if (decode.op3.source_reg == CG)
						set.source = constant2;
					else
						set.source = indirect_register_mode;
					break;
				case 3:
					if (decode.op3.source_reg == PC)
						set.source = immediate_mode;
					else if (decode.op3.source_reg == SR)
						set.source = constant8;
					else if (decode.op3.source_reg == CG)
						set.source = constantneg1;
					else
						set.source = indirect_autoincrement;
					break;
				default: break;
			}
			if (decode.op3.ad == 0)
			{
				set.destination = register_mode;
			}
			else
			{
				if (decode.op3.dest_reg == 0)
					set.destination = symbolic_mode;
				else if (decode.op3.dest_reg == 2)
					set.destination = absolute_mode;
				else if (decode.op3.dest_reg != 3)
					set.destination = indexed_mode;
				else
					set.destination = error;
			}
			set.op[2].instruction = (mnem)(decode.op3.opcode + (MOV-4));
			set.op[2].source_reg = decode.op3.source_reg;
			set.op[2].dest_reg = decode.op3.dest_reg;
			if (set.op[2].instruction == MOV)
			{
				if (set.op[2].source_reg == set.op[2].dest_reg && set.source == register_mode
					&& set.destination == register_mode)
				{
					set.op[2].instruction = NOP;
				}
				else if (set.source == constant0 && 
						 set.destination == register_mode && set.op[2].dest_reg == SR)
				{
					set.op[2].instruction = NOP;
				}
				if (set.source == indirect_autoincrement && set.op[2].source_reg == SP &&
					set.destination == register_mode && set.op[2].dest_reg == PC)
				{
					set.op[2].instruction = RET;
				}
				else if (set.source == indirect_autoincrement && set.op[2].source_reg == SP)
				{
					set.op[2].instruction = POP;
				}
				if (set.destination == register_mode && set.op[2].dest_reg == PC)
				{
					set.op[2].instruction = BR;
				}
			}
			else if (set.op[2].instruction == BIC || set.op[2].instruction == BIS)
			{
				if (set.destination == register_mode && set.op[2].dest_reg == SR
					&& set.op[2].instruction == BIC)
				{
					if (set.source == constant1)
						set.op[2].instruction = CLRC;
					else if (set.source == constant2)
						set.op[2].instruction = CLRZ;
					else if (set.source == constant4)
						set.op[2].instruction = CLRN;
					else if (set.source == constant8)
						set.op[2].instruction = DINT;
				}
				else if (set.destination == register_mode && set.op[2].dest_reg == SR
					&& set.op[2].instruction == BIS)
				{
					if (set.source == constant1)
						set.op[2].instruction = SETC;
					else if (set.source == constant2)
						set.op[2].instruction = SETZ;
					else if (set.source == constant4)
						set.op[2].instruction = SETN;
					else if (set.source == constant8)
						set.op[2].instruction = EINT;
				}
			}
			else  if (set.op[2].instruction == ADD)
			{
				if (set.source == register_mode && set.destination == register_mode &&
					set.op[2].source_reg == set.op[2].dest_reg)
				{
					set.op[2].instruction = RLA;
				}
				else if (set.source == constant1)
				{
					set.op[2].instruction = INC;
				}
				else if (set.source == constant2)
				{
					set.op[2].instruction = INCD;
				}
			}
			else if (set.op[2].instruction == ADDC)
			{
				if (set.source == register_mode && set.destination == register_mode &&
					set.op[2].source_reg == set.op[2].dest_reg)
				{
					set.op[2].instruction = RLC;
				}
				else if (set.source == constant0)
				{
					set.op[2].instruction = ADC;
				}
			}
			else if (set.op[2].instruction == SUBC || set.op[2].instruction == XOR ||
					 set.op[2].instruction == MOV || set.op[2].instruction == CMP ||
					 set.op[2].instruction == SUB || set.op[2].instruction == DADD)
			{
				if (set.source == constantneg1 && set.op[2].instruction == XOR)
				{
					set.op[2].instruction = INV;
				}
				else if (set.source == constant0 && set.op[2].instruction == MOV)
				{
					set.op[2].instruction = CLR;
				}
				else if (set.source == constant0 && set.op[2].instruction == CMP)
				{
					set.op[2].instruction = TST;
				}
				else if (set.source == constant1 && set.op[2].instruction == SUB)
				{
					set.op[2].instruction = DEC;
				}
				else if (set.source == constant2 && set.op[2].instruction == SUB)
				{
					set.op[2].instruction = DECD;
				}
				else if (set.source == constant0 && set.op[2].instruction == DADD)
				{
					set.op[2].instruction = DADC;
				}
				else if (set.source == constant0 && set.op[2].instruction == SUBC)
				{
					set.op[2].instruction = SBC;
				}
			}
			break;
		default: set.op[set.optype-1].instruction = ERROR;
			break;
	}
	switch (set.source)
	{
		case register_mode:
		case indirect_register_mode:
		case indirect_autoincrement:
			if (set.destination == register_mode || set.optype == 1)
				amount_used = 2;
			else
				amount_used = 4;
			break;
		case immediate_mode:
		case indexed_mode:
		case symbolic_mode:
		case absolute_mode:
			if (set.destination == register_mode || set.optype == 1)
				amount_used = 4;
			else
				amount_used = 6;
			break;
		default: 
			amount_used = 2;
			break;
	}
	return amount_used;
}

int output(uint16_t num[3], uint used, instruction& set)
{
//	swab(&code[(6-bytes_used)/2],&code[(6-bytes_used)/2], bytes_used);
	for (int x = 0; x < used/2; x++)
	{
		cout << hex << num[x] << " ";
	}
	cout << "\t" << dec << map[set.op[set.optype-1].instruction-1];
	switch(set.optype)
	{
		case 1:
		case 3:
			switch(set.source)
			{
				case register_mode:
					if (set.optype == 3)
						cout << "\tr" << set.op[2].source_reg;	
					else
						cout << "\tr" << set.op[0].source_reg;
					break;
				case indexed_mode:
					if (set.optype == 3)
						cout << "\tx(r" << set.op[2].source_reg << ")";	
					else
						cout << "\tx(r" << set.op[0].source_reg << ")";
					break;
				case symbolic_mode:
					cout << "\t0x" << num[1];
					break;
				case absolute_mode:
					cout << "\t&0x" << num[1];
					break;
				case indirect_register_mode:
					if (set.optype == 3)
						cout << "\t@r" << set.op[2].source_reg;
					else
						cout << "\t@r" << set.op[0].source_reg;
					break;
				case indirect_autoincrement:
					if (set.optype == 3)
						cout << "\t@r" << set.op[2].source_reg << "+";
					else
						cout << "\t@r" << set.op[0].source_reg << "+";
					break;
				case immediate_mode:
					cout << "\t#0x" << num[1];
					break;
				case constantneg1:
					cout << "\t#-1";
					break;
				case constant0:
					cout << "\t#0";
					break;
				case constant1:
					cout << "\t#1";
					break;
				case constant2:
					cout << "\t#2";
					break;
				case constant4:
					cout << "\t#4";
					break;
				case constant8:
					cout << "\t#8";
					break;
				case error:
					cout << "error, op not valid.";
					break;
				default: cout << "error, op not valid.";
					break;
			}
			if (set.optype == 3)
			{
				switch(set.destination)
				{
					case register_mode:
						cout << ",\tr" << set.op[2].dest_reg;
						break;
					case indexed_mode:
						cout << ",\tx(r" << set.op[2].dest_reg << ")";
						break;
					case symbolic_mode:
						cout << ",\t0x" << num[1];
						break;
					case absolute_mode:
						cout << ",\t&0x" << num[1];
						break;
					default: cout << ", \terror, op not valid.";
						break;
				}
			}
			cout << endl;
			break;
		case 2:
			cout << &num[0] << endl;
		default: break;
	}
	return 0;
}

int main()
{
	ifstream textfile ("hi.bin", ios::binary);
	textfile.seekg(0,textfile.end);
	size_t size = textfile.tellg();
	textfile.seekg(0, textfile.beg);
	if (size <= 1)
	{
		cerr << "Invalid\n";
		return -1;
	}
	uint16_t code[3] = {0,0,0};
	uint16_t bytes_used = 0;
	textfile.read((char*)&code, sizeof(code));
	int x = 1;
	while (size >= 2)
	{
		instruction set;
		memset(&set, 0, sizeof(set));
		bytes_used = msp430Decompose(code, set);
		output(code, bytes_used, set);
		size -= bytes_used;
		memmove(&code[0], &code[bytes_used/2], 6-bytes_used);
		textfile.read((char*)&code[(6-bytes_used)/2], bytes_used);
	}
	return 0;
}






