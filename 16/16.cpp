#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>

// https://adventofcode.com/2018/day/16

// virtual machine instruction instruction
struct Instruction
{
	int opcode;
	int a;
	int b;
	int c;
};

// sample table entry
struct Sample
{
	int before[4];
	Instruction instruction;
	int after[4];

};

// function prototype for virtual machine instructions
typedef void(*Op)(int registers[], int a, int b, int c);

// functions implementing virtual machine instructions
void Op_addr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] + registers[b];
}
void Op_addi(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] + b;
}
void Op_mulr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] * registers[b];
}
void Op_muli(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] * b;
}
void Op_banr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] & registers[b];
}
void Op_bani(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] & b;
}
void Op_borr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] | registers[b];
}
void Op_bori(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] | b;
}
void Op_setr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a];
}
void Op_seti(int registers[], int a, int b, int c)
{
	registers[c] = a;
}
void Op_gtir(int registers[], int a, int b, int c)
{
	registers[c] = a > registers[b];
}
void Op_gtri(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] > b;
}
void Op_gtrr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] > registers[b];
}
void Op_eqir(int registers[], int a, int b, int c)
{
	registers[c] = a == registers[b];
}
void Op_eqri(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] == b;
}
void Op_eqrr(int registers[], int a, int b, int c)
{
	registers[c] = registers[a] == registers[b];
}

// table of virtual machine instructions
// (with opcode assignment to be determined)
Op const op_table[] =
{
	Op_addr,
	Op_addi,
	Op_mulr,
	Op_muli,
	Op_banr,
	Op_bani,
	Op_borr,
	Op_bori,
	Op_setr,
	Op_seti,
	Op_gtir,
	Op_gtri,
	Op_gtrr,
	Op_eqir,
	Op_eqri,
	Op_eqrr,
};
static_assert(sizeof(op_table) / sizeof(op_table[0]) == 16, "incorrect number of opcodes");

// read sample table
void ReadSamples(std::vector<Sample> &output, std::istream &input)
{
	char line[256];

	for (;;)
	{
		Sample sample = { 0 };
		if (!input.getline(line, sizeof(line)))
			break;
		if (sscanf_s(line, "Before: [%d, %d, %d, %d]", &sample.before[0], &sample.before[1], &sample.before[2], &sample.before[3]) != 4)
			break;
		if (!input.getline(line, sizeof(line)))
			break;
		if (sscanf_s(line, "%d %d %d %d", &sample.instruction.opcode, &sample.instruction.a, &sample.instruction.b, &sample.instruction.c) != 4)
			break;
		if (!input.getline(line, sizeof(line)))
			break;
		if (sscanf_s(line, "After: [%d, %d, %d, %d]", &sample.after[0], &sample.after[1], &sample.after[2], &sample.after[3]) != 4)
			break;
		if (!input.getline(line, sizeof(line)))
			break;
		if (line[0] != '\0')
			break;
		output.push_back(sample);
	}
}

// PART 1
void Part1(std::vector<Sample> const &samples)
{
	// number of samples with 3+ matches
	int sample_count = 0;

	// for each sample...
	for (Sample const &sample : samples)
	{
		// number of matching
		int matches = 0;

		// for each virtual machine instruction
		for (Op op : op_table)
		{
			int registers[4];

			// set registers to the sample's "before" state
			memcpy(registers, sample.before, sizeof(registers));

			// execute the candidate virtual machine instruction
			op(registers, sample.instruction.a, sample.instruction.b, sample.instruction.c);

			// check registers against the sample's "after" state
			if (memcmp(registers, sample.after, sizeof(registers)) == 0)
			{
				// result matches
				++matches;
			}
		}

		// count the sample if three or more instructions match
		sample_count += (matches >= 3);
	}

	std::cout << "Part 1: " << sample_count << " samples behave like 3+ opcodes\n";
}

// print a candidate matrix
void PrintCandidateMatrix(bool (& candidate_matrix)[16][16])
{
	for (int opcode = 0; opcode < 16; ++opcode)
	{
		std::cout << "Opcode " << std::setw(2) << opcode << ": ";
		for (int index = 0; index < 16; ++index)
		{
			std::cout << (candidate_matrix[opcode][index] ? 'Y' : '.');
		}
		std::cout << "\n";
	}
}

// build the opcode table from the sample table
void BuildOpTable(std::vector<Op> &output, std::vector<Sample> const &samples)
{
	output.clear();
	output.resize(16, nullptr);

	// matrix of candidate pairs of opcode and index
	bool candidate_matrix[16][16];

	// start with all candidates potentially valid
	memset(candidate_matrix, true, sizeof(candidate_matrix));

	// for each sample table entry...
	for (Sample const &sample : samples)
	{
		// for each virtual machine instruction...
		Op op_match = nullptr;
		for (size_t index = 0; index < 16; ++index)
		{
			Op op = op_table[index];

			int registers[4];

			// set registers to the sample's "before" state
			memcpy(registers, sample.before, sizeof(registers));

			// execute the candidate virtual machine instruction
			op(registers, sample.instruction.a, sample.instruction.b, sample.instruction.c);

			// check registers against the sample's "after" state
			if (memcmp(registers, sample.after, sizeof(registers)) != 0)
			{
				// candidate instruction is inconsistent with the opcode's behavior
				candidate_matrix[sample.instruction.opcode][index] = false;
			}
		}
	}

#ifdef DEBUG_PRINT
	PrintCandidateMatrix(candidate_matrix);
#endif

	uint16_t unassigned_opcodes = UINT16_MAX, unassigned_indices = UINT16_MAX;

	// perform assignment
	auto Assign = [&output, &unassigned_opcodes, &unassigned_indices, &candidate_matrix](int opcode, int index)
	{
		// assign the index to the opcode
		output[opcode] = op_table[index];

		// remove the opcode from unassigned opcodes
		unassigned_opcodes &= ~(1 << opcode);

		// remove the index from unassigned indices
		unassigned_indices &= ~(1 << index);

		// remove the index from the candidate matrix
		for (int o = 0; o < 16; ++o)
		{
			candidate_matrix[o][index] = false;
		}

		// remove the opcode from the candidate matrix
		for (int i = 0; i < 16; ++i)
		{
			candidate_matrix[opcode][i] = false;
		}
	};

	// while there are unassigned opcodes and indices...
	while (unassigned_opcodes && unassigned_indices)
	{
		// search the candidate matrix for an opcode with one candidate index
		for (int opcode = 0; opcode < 16; ++opcode)
		{
			if (unassigned_opcodes & (1 << opcode))
			{
				// find the candidate index (if any)
				int index = -1;
				for (int i = 0; i < 16; ++i)
				{
					if (candidate_matrix[opcode][i])
					{
						if (index < 0)
						{
							index = i;
						}
						else
						{
							// multiple candidates
							index = -1;
							break;
						}
					}
				}
				if (index >= 0)
				{
					// assign the index to the opcode
					std::cout << "opcode " << opcode << " = index " << index << "\n";
					Assign(opcode, index);
#ifdef DEBUG_PRINT
					PrintCandidateMatrix(candidate_matrix);
#endif
				}
			}
		}

		// search the candidate matrix for an index with one candidate opcode
		for (int index = 0; index < 16; ++index)
		{
			if (unassigned_indices & (1 << index))
			{
				// find the candidate opcode (if any)
				int opcode = -1;
				for (int o = 0; o < 16; ++o)
				{
					if (candidate_matrix[o][index])
					{
						if (opcode < 0)
						{
							opcode = o;
						}
						else
						{
							// multiple candidates
							opcode = -1;
							break;
						}
					}
				}
				if (opcode >= 0)
				{
					// assign the index to the opcode
					std::cout << "index " << index << " = opcode " << opcode << "\n";
					Assign(opcode, index);
#ifdef DEBUG_PRINT
					PrintCandidateMatrix(candidate_matrix);
#endif
				}
			}
		}
	}
}

// read program instructions
void ReadProgram(std::vector<Instruction> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Instruction instruction;
		if (sscanf_s(line, "%d %d %d %d", &instruction.opcode, &instruction.a, &instruction.b, &instruction.c) == 4)
		{
			output.push_back(instruction);
		}
	}
}

// run a program
void RunProgram(int registers[], std::vector<Instruction> &instructions, std::vector<Op> const &ops)
{
	for (Instruction const &instruction : instructions)
	{
		ops[instruction.opcode](registers, instruction.a, instruction.b, instruction.c);
	}
}

// PART 2
void Part2(std::vector<Instruction> &instructions, std::vector<Op> const &ops)
{
	int registers[4] = { 0, 0, 0, 0 };
	RunProgram(registers, instructions, ops);
	std::cout << "Part 2: register 0 contains " << int(registers[0]) << "\n";
}

int main()
{
	// read sample table
	std::vector<Sample> samples;
	ReadSamples(samples, std::cin);

	Part1(samples);

	// build the opcode table
	std::vector<Op> ops;
	BuildOpTable(ops, samples);

	// read the program
	std::vector<Instruction> instructions;
	ReadProgram(instructions, std::cin);

	Part2(instructions, ops);

	return 0;
}