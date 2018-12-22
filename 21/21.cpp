#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <unordered_set>

// https://adventofcode.com/2018/day/21

using OpcodeType = uint32_t;
using RegisterType = uint32_t;

// function prototype for virtual machine instructions
typedef void(*Op)(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c);

// functions implementing virtual machine instructions
void Op_addr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] + registers[b];
}
void Op_addi(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] + b;
}
void Op_mulr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] * registers[b];
}
void Op_muli(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] * b;
}
void Op_banr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] & registers[b];
}
void Op_bani(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] & b;
}
void Op_borr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] | registers[b];
}
void Op_bori(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] | b;
}
void Op_setr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a];
}
void Op_seti(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = a;
}
void Op_gtir(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = a > registers[b];
}
void Op_gtri(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] > b;
}
void Op_gtrr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] > registers[b];
}
void Op_eqir(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = a == registers[b];
}
void Op_eqri(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] == b;
}
void Op_eqrr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] == registers[b];
}

// additional opcodes
void Op_divr(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] / registers[b];
}
void Op_divi(RegisterType registers[], RegisterType a, RegisterType b, RegisterType c)
{
	registers[c] = registers[a] / b;
}

// virtual machine instruction instruction
struct Instruction
{
	OpcodeType op;
	RegisterType a;
	RegisterType b;
	RegisterType c;
};


// table of virtual machine instructions
// (with opcode assignment to be determined)
using OpTableEntry = std::pair<std::string, Op>;
using OpTable = std::vector<OpTableEntry>;
OpTable op_table =
{
	{ "addr", Op_addr },
	{ "addi", Op_addi },
	{ "mulr", Op_mulr },
	{ "muli", Op_muli },
	{ "banr", Op_banr },
	{ "bani", Op_bani },
	{ "borr", Op_borr },
	{ "bori", Op_bori },
	{ "setr", Op_setr },
	{ "seti", Op_seti },
	{ "gtir", Op_gtir },
	{ "gtri", Op_gtri },
	{ "gtrr", Op_gtrr },
	{ "eqir", Op_eqir },
	{ "eqri", Op_eqri },
	{ "eqrr", Op_eqrr },
	// additional opcodes
	{ "divr", Op_divr },
	{ "divi", Op_divi },
};

// read program instructions
int ReadProgram(std::vector<Instruction> &output, std::istream &input)
{
	int ip_register = -1;

	char line[256];

	// get the ip register from the first line
	if (input.getline(line, sizeof(line)))
	{
		int ip;
		if (sscanf_s(line, "#ip %d", &ip) == 1)
			ip_register = ip;
	}

	// read the instructions and parameters
	while (input.getline(line, sizeof(line)))
	{
		Instruction instruction;
		char op_name[16];
		if (sscanf_s(line, "%s %d %d %d", op_name, uint32_t(sizeof(op_name)), &instruction.a, &instruction.b, &instruction.c) == 4)
		{
			OpTable::iterator entry = std::find_if(op_table.begin(), op_table.end(), [&op_name](OpTableEntry const &entry) -> bool { return entry.first == op_name; });
			instruction.op = OpcodeType(entry - op_table.begin());	// save the opcode index
			output.push_back(instruction);
		}
	}

	return ip_register;
}

// write out the registers
std::ostream & operator<< (std::ostream& stream, RegisterType registers[])
{
	return stream << "["
		<< registers[0] << ", "
		<< registers[1] << ", "
		<< registers[2] << ", "
		<< registers[3] << ", "
		<< registers[4] << ", "
		<< registers[5]
		<< "]";
}

// run a program
RegisterType RunProgram(RegisterType registers[], int ip_register, std::vector<Instruction> const &instructions, int part)
{
	RegisterType &ip = registers[ip_register];

	std::unordered_set<RegisterType> visited;

	RegisterType result = 0;

	while (ip >= 0 && ip < instructions.size())
	{
		Instruction const &instruction = instructions[ip];
		OpTableEntry const &op_entry = op_table[instruction.op];

		// if this is the instruction comparing against the input value in r0...
		if (instruction.op == 15 && (instruction.a == 0 || instruction.b == 0))
		{
			RegisterType value = (instruction.a == 0) ? registers[instruction.b] : registers[instruction.a];
			if (part == 1)
			{
				// part 1: want the first value compared
				result = value;
				break;
			}
			else
			{
				// part 2: want the last value before the sequence repeats
				if (visited.find(value) == visited.end())
				{ 
					visited.insert(value);
					result = value;
				}
				else
				{
					break;
				}
			}
		}

		(op_entry.second)(registers, instruction.a, instruction.b, instruction.c);
		++ip;
	}

	return result;
}

// PART 1
void Part1(int ip_register, std::vector<Instruction> const &instructions)
{
	// initial state of the registers
	RegisterType registers[6] = { 0, 0, 0, 0, 0, 0 };

	// run the program
	RegisterType result = RunProgram(registers, ip_register, instructions, 1);

	// print the result
	std::cout << "Part 1: first value compared is " << result << "\n";
}

// PART 2
void Part2(int ip_register, std::vector<Instruction> const &instructions)
{
	// initial state of the registers
	RegisterType registers[6] = { 0, 0, 0, 0, 0, 0 };

	// run the program
	RegisterType result = RunProgram(registers, ip_register, instructions, 2);

	// print the result
	std::cout << "Part 2: last value before repeat is " << result << "\n";
}

int main()
{
	// read the instruction pointer register number and the program
	std::vector<Instruction> instructions;
	int ip_register = ReadProgram(instructions, std::cin);

	Part1(ip_register, instructions);
	Part2(ip_register, instructions);

	return 0;
}