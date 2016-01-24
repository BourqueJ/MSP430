#include <iostream>
#include <string>
#include <fstream>
using namespace std;

enum mnem {
	UNDEFINED = 0, RRC, SWRB, RRA,
	SXT, PUSH, CALL, RETI,
	JNE, JEQ, JNC, JC,
	JN, JGE, JL, JMP,
	MOV, ADD, ADDC, SUBC,
	SUB, CMP, DADD, BIT,
	BIC, BIS, XOR, AND,
	NOP
};

enum registers {
	PC, SP, CG1, CG2,
	R4, R5, R6, R7,
	R8, R9, R10, R11,
	R12, R13, R14, R15
};

enum types {
	dest, src, dest_reg,
	src_reg, label
};

enum addressing_mode
{
	register_mode, indexed_mode, symbolic_mode, absolute_mode,
	indirect_register_mode, indirect_autoincrement, immediate_mode,
	constant[6] = {-1, 0, 1, 2, 4, 8}, error
};

struct instruction
{
	struct operand
	{
		types type;
		registers reg;
		uint16_t imm;
	}op[3];
	mnem instruction;
	addressing_mode mode;
}set;

int msp430Decompose(int16_t value)
{
	union {
		uint16_t full;
		struct 
		{
			registers dest_reg:4;
			uint16_t ad:2;
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
	decode.full = value;
	enum mnem2{
		MOV = 4, ADD, ADDC, SUBC,
		SUB, CMP, DADD, BIT,
		BIC, BIS, XOR, AND
	};
	switch ((decode.op3.opcode))
	{
		case 0: cout << "error\n";
			break;
		case 1:
			switch(decode.op3.as)
			{
				case 0:
					if (decode.op3.ad == 0)
					{
						set.addressing_mode = register_mode;
					}
					else
					{
						set.addressing_mode = error;
					}
					break;
				case 1:
					if (decode.op3.ad == 1)
					{
						if (decode.op3.dest_reg != PC && decode.op3.dest_reg != SR &&
							decode.op3.dest_reg != CG1)
						{
							set.addressing_mode = indexed_mode;
						}
						if (decode.op3.dest_reg == PC)
						{
							set.addressing_mode = symbolic_mode;
						}
						if (decode.op3.dest_reg == SR)
						{
							set.addressing_mode = absolute_mode;
						}
					}
			}
			set.instruction = decode.op1.opcode;
			set.op[0].type = dest_reg;
			set.op[0].reg = decode.op1.dest_reg;
			break;
		case 2:
		case 3:
			set.instruction = (mnem)(decode.op2.condition + RETI);
			set.op[0].type = label;
			set.op[0].imm = decode.full;
			break;
		case MOV:
			if (decode.op3.source_reg == decode.op3.dest_reg && decode.op3.source_reg >= 3 &&
				decode.op3.opcode == MOV)
			{
				set.instruction = NOP;
				break;
			}
			else if (decode.op3.opcode == MOV && decode.op3.as == 0 && decode.op3.source_reg == 3)
			{
				set.instruction = CLR;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
				break;
			}
			else if (decode.op3.as == 3 && decode.op3.source_reg != 0 &&
					 decode.op3.source_reg != 2 && decode.op3.source_reg != 3)
			{
				if (decode.op3.dest_reg == PC)
				{
					set.instruction = RET;
					break;
				}
				else
				{
					set.instruction = POP;
					set.op[0].type = dest_reg;
					set.ep[0].reg = decode.op3.dest_reg;
					break;
				}
			}
			else if (decode.op3.opcode == MOV && decode.op3.source_reg == PC &&
					 decode.op3.source_reg == PC)
			{
				set.instruction = BR;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
				break;
			}

		case ADD:
			if (decode.op3.opcode == ADD && (decode.op3.as == 1 || decode.op3.as == 2)
				&& decode.op3.source_reg == 3)
			{
				if (decode.op3.as == 1)
				{
					set.instruction = INC;
					set.op[0].type = dest_reg;
					set.op[0].reg = decode.op3.dest_reg;
					break;
				}
				else
				{
					set.instruction = INCD;
					set.op[0].type = dest_reg;
					set.op[0].reg = decode.op3.dest_reg;
					break;
				}
			}
		case ADDC:
			if (decode.op3.source_reg == decode.op3.dest_reg && (decode.op3.opcode == ADD ||
				decode.op3.opcode == ADDC))
			{
				if (decode.op3.opcode == ADD)
					set.instruction = RLA;
				else
					set.instruction = RLC;

				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
				break;
			}
			else if (decode.op3.opcode == ADDC && decode.op3.as == 0 && decode.op3.source_reg == 3)
			{
				set.instruction = ADC;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
			}

		case SUBC:
			if (decode.op3.opcode == SUBC && decode.op3.as == 0 && decode.op3.source_reg == 3)
			{
				set.instruction = SBC;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
			}
		case SUB:
			if (decode.op3.opcode == SUB && (decode.op3.as == 1 || decode.op3.as == 2)
				&& decode.op3.source_reg == 3)
			{
				if (decode.op3.as == 1)
				{
					set.instruction = DEC;
					set.op[0].type = dest_reg;
					set.op[0].reg = decode.op3.dest_reg;
					break;
				}
				else
				{
					set.instruction = DECD;
					set.op[0].type = dest_reg;
					set.op[0].reg = decode.op3.dest_reg;
					break;
				}
			}
		case CMP:
			if (decode.op3.opcode == CMP && decode.op3.as == 0 && decode.op3.source_reg == 3)
			{
				set.instruction = TST;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
				break;
			}
		case DADD:
			if (decode.op3.opcode == DADD && decode.op3.as == 0 && decode.op3.source_reg == 3)
			{
				set.instruction = DADC;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
			}
		case BIT:
		case BIC:
		case BIS:
			if (decode.op3.dest_reg == SP && (decode.op3.opcode == BIC || decode.op3.opcode == BIS))
			{
				switch(decode.op3.as)
				{
					case 1:
						if (decode.op3.source_reg == 3)
						{
							if (decode.op3.opcode == BIC)
								set.instruction = CLRC;
							else
								set.instruction = SETC;
						}
						break;
					case 2:
						if (decode.op3.source_reg == 2)
						{
							if (decode.op3.opcode == BIC)
								set.instruction = CLRN;
							else
								set.instruction = SETN;
						}
						else if (decode.op3.source_reg == 3)
						{
							if (decode.op3.opcode == BIC)
								set.instruction = CLRZ;
							else
								set.instruction = SETZ;
						}
						break;
					case 3:
						if (decode.op3.source_reg == 2)
						{
							if (decode.op3.opcode == BIC)
								set.instruction = DINT;
							else
								set.instruction = EINT;
						}
						break;
					default: break;

				}
				break;
			} 
		case XOR:
			if (decode.op3.opcode == XOR && decode.op3.as == 3 && decode.op3.source_reg == 3)
			{
				set.instruction = INV;
				set.op[0].type = dest_reg;
				set.op[0].reg = decode.op3.dest_reg;
				break;
			}
		case AND:
			set.instruction = (mnem)(decode.op3.opcode + JMP);
			set.op[0].type = src_reg;
			set.op[0].reg = decode.op3.source_reg;
			set.op[1].type = dest_reg;
			set.op[1].reg = decode.op3.dest_reg;
			
			break;
		default: cerr << "ERROR 1\n"; return -1;
			break;
	}
		return 0;
}

int main()
{
	ifstream textfile ("helloworld.txt");
	textfile.seekg(0,textfile.end);
	size_t size = textfile.tellg();
	textfile.seekg(0, textfile.beg);
	if (size == 0)
	{
		cerr << "Invalid\n";
		return -1;
	}
	int16_t code = 0;
	for (int x = 0; x < size/2; x += 2)
	{
		textfile.read((char*)&code, 2);
		msp430Decompose(code);
	}
}









