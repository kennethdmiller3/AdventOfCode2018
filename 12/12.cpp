#include <iostream>
#include <vector>

// https://adventofcode.com/2018/day/12

struct State
{
	std::vector<bool> occupied;
	int lower = 0;
	int upper = 0;
	int64_t offset = 0;
};

void UpdateBounds(State &state)
{
	// compute lower bound (minus offset)
	for (int i = 0; i < state.occupied.size(); ++i)
	{
		if (state.occupied[i])
		{
			state.lower = i;
			break;
		}
	}

	// compute upper bound (minus offset)
	for (int i = 0; i < state.occupied.size(); ++i)
	{
		if (state.occupied[i])
		{
			state.upper = i;
		}
	}
}

void PrintState(int64_t generation, State const &state)
{
	std::cout << generation << ": ";
	for (int i = 0; i < int(state.occupied.size()); ++i)
	{
		std::cout << (state.occupied[i] ? '#' : '.');
	}
	std::cout << "\n";
}

// read in the initial state
void ReadInitial(State &output, std::istream &input)
{
	char line[256];
	input.getline(line, sizeof(line));
	char state[256];
	sscanf_s(line, "initial state: %s", state, unsigned int(sizeof(state)));
	for (char const element : state)
	{
		if (element == '\0')
			break;
		output.occupied.push_back(element == '#');
	}
	output.offset = 0;
	UpdateBounds(output);
}

// read in the patterns
void ReadPatterns(std::vector<bool> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		char iLL, iL, iC, iR, iRR, o;
		if (sscanf_s(line, "%c%c%c%c%c => %c", &iLL, 1, &iL, 1, &iC, 1, &iR, 1, &iRR, 1, &o, 1) == 6)
		{
			output[((iLL == '#') << 0) | ((iL == '#') << 1) | ((iC == '#') << 2) | ((iR == '#') << 3) | ((iRR == '#') << 4)] = (o == '#');
		}
	}
}

// perform one iteration
void Iterate(State &output, State const &input, std::vector<bool> const &patterns)
{
	// resize the output to hold the largest possible result
	output.occupied.resize(input.upper - input.lower + 1 + 2 + 2);

	// for each output location...
	for (int i = input.lower - 2; i < input.upper + 2; ++i)
	{
		// compute the new output based on inputs
		const bool iLL = i >= input.lower + 2 && i <= input.upper + 2 && input.occupied[i - 2];
		const bool iL =  i >= input.lower + 1 && i <= input.upper + 1 && input.occupied[i - 1];
		const bool iC =  i >= input.lower + 0 && i <= input.upper     && input.occupied[i    ];
		const bool iR =  i >= input.lower - 1 && i <= input.upper - 1 && input.occupied[i + 1];
		const bool iRR = i >= input.lower - 2 && i <= input.upper - 2 && input.occupied[i + 2];
		output.occupied[i - input.lower + 2] = patterns[(iLL << 0) | (iL << 1) | (iC << 2) | (iR << 3) | (iRR << 4)];
	}

	// update the bounds and offset
	UpdateBounds(output);
	output.offset = input.offset + input.lower - 2;
}

// run the simulation from an initial state for a specified number of generations
void RunSimulation(State &result, State const &initial, std::vector<bool> const &patterns, int64_t generations)
{
	State next;

	// print initial state
	PrintState(0, initial);

	// start with the initial state
	result = initial;

	// for each generation...
	for (int64_t generation = 1; generation <= generations; ++generation)
	{
		// get new state
		Iterate(next, result, patterns);

		// print new state
		PrintState(generation + 1, next);

		// exchange states
		std::swap(next, result);

		// if the occupied locations reach equilibrium...
		if (next.lower == result.lower && next.upper == result.upper && next.occupied == result.occupied)
		{
			// compute what the result would be at the end
			std::cout << "reached equilibrium at generation " << generation << "\n"; 
			std::cout << "each generation changes offset by " << result.offset - next.offset << "\n";
			std::cout << "applying offset for remaining " << generations - generation << " generations\n";
			result.offset += (result.offset - next.offset) * (generations - generation);
			break;
		}
	}
}

int64_t ComputeSum(State const &state)
{
	// sum of the indices of the occupied locations
	int64_t sum = 0;
	for (int i = state.lower; i <= state.upper; ++i)
	{
		if (state.occupied[i])
			sum += i + state.offset;
	}
	return sum;
}

void Part1(State const &initial, std::vector<bool> const &patterns)
{
	// run simulation for 20 generations
	State result;
	RunSimulation(result, initial, patterns, 20);
	int64_t sum = ComputeSum(result);
	std::cout << "Part 1: plant position sum is " << sum << "\n";
}

void Part2(State const &initial, std::vector<bool> const &patterns)
{
	// run simulation for 50 billion generations
	State result;
	RunSimulation(result, initial, patterns, 50000000000);
	int64_t sum = ComputeSum(result);
	std::cout << "Part 2: plant position sum is " << sum << "\n";
}

int main()
{
	// read initial state
	State initial;
	ReadInitial(initial, std::cin);

	// read patterns
	std::vector<bool> patterns(1 << 5, false);
	ReadPatterns(patterns, std::cin);

	Part1(initial, patterns);
	Part2(initial, patterns);

	return 0;
}