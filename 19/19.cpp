#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

// https://adventofcode.com/2018/day/19

// the program supplied as puzzle input generates a value in registers[4] based on the starting value of registers[1]
// then computes the sum of the factors of registers[4] but uses an extremely inefficient O(N^2) algorithm to do so

// for the registers[0] == 0 case (Part 1): registers[4] = 926
// factors are 1, 2, 463, 926
// sum of factors is 1392

// for the registers[0] == 0 case (Part 2): registers[4] = 10551326
// factors are 1, 2, 5275663, 10551326
// sum of factors is 15826992

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

// virtual machine instruction instruction
struct Instruction
{
	Op op;
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
			instruction.op = entry->second;
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
void RunProgram(RegisterType registers[], int ip_register, std::vector<Instruction> const &instructions)
{
	RegisterType &ip = registers[ip_register];

	while (ip >= 0 && ip < instructions.size())
	{
		Instruction const &instruction = instructions[ip];
		instruction.op(registers, instruction.a, instruction.b, instruction.c);
		++ip;
	}
}

// PART 1
void Part1(int ip_register, std::vector<Instruction> const &instructions)
{
	std::cout << "Part 1:\n";

	// initial state of the registers
	RegisterType registers[6] = { 0, 0, 0, 0, 0, 0 };

	// run the program
	RunProgram(registers, ip_register, instructions);

	// print final state of the registers
	std::cout << "Registers: " << registers << "\n";

	// print the result
	std::cout << "Register 0 contains " << int(registers[0]) << "\n";
}

// PART 2
void Part2(int ip_register, std::vector<Instruction> const &instructions)
{
	std::cout << "Part 2:\n";

	// initial state of the registers
	RegisterType registers[6] = { 1, 0, 0, 0, 0, 0 };

#if 1
	// actually running the supplied program takes far too long
	// so here's an equivalent that computes the same result

	// "work smarter, not harder!"

	// set registers[4] to the value that the program would generate
	registers[4] = registers[0] ? 10551326 : 926;

	// clear the accumulator
	registers[0] = 0;

	// for each potential factor of the value in registers[4]...
	for (registers[2] = 1; registers[2] <= registers[4]; ++registers[2])
	{
		// determine if the value is a factor

#if 1
		// much faster implementation that yields O(N) runtime
		if (registers[4] % registers[2] == 0)
		{
			registers[0] += registers[2];
		}
#else
		// equivalent implementation to the supplied program
		for (registers[5] = 1; registers[5] <= registers[4]; ++registers[5])
		{
			if (registers[2] * registers[5] == registers[4])
			{
				registers[0] += registers[2];
			}

#if 1
			// performance fix:
			// the product gets larger each iteration of the inner loop
			// so stop once the product exceeds the value in registers[4]
			if (registers[2] * registers[5] >= registers[4])
			{
				break;
			}
#endif
		}
#endif
	}

	// set the inner loop variable to the value it would have at the end
	registers[5] = registers[4] + 1;

	// set the instruction pointer to the value it woud have when the program terminates
	registers[3] = 257;

#else

	// actually running the program takes a VERY long time! (days?)
	RunProgram(registers, ip_register, instructions);

#endif

	// print final state of the registers
	std::cout << "Registers: " << registers << "\n";

	std::cout << "Register 0 contains " << int(registers[0]) << "\n";
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