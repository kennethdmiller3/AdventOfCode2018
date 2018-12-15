#include <iostream>
#include <string>

// https://adventofcode.com/2018/day/14

// print the state in the format on the puzzle entry page
void PrintState(std::string const &state, int index1, int index2)
{
	for (int i = 0; i < state.size(); ++i)
	{
		if (i == index1)
		{
			std::cout << "(" << state[i] << ")";
		}
		else if (i == index2)
		{
			std::cout << "[" << state[i] << "]";
		}
		else
		{
			std::cout << " " << state[i] << " ";
		}
	}
	std::cout << "\n";
}

// PART 1
void Part1(int count)
{
	// run until the state length is count + 10
	std::string state = "37";
	int index1 = 0, index2 = 1;
	for(;;)
	{
		int score1 = state[index1] - '0';
		int score2 = state[index2] - '0';
		int sum = score1 + score2;
		if (sum >= 10)
		{
			state.push_back('0' + sum / 10);
			if (state.length() >= count + 10)
				break;
			state.push_back('0' + sum % 10);
		}
		else
		{
			state.push_back('0' + sum);
		}
		if (state.length() >= count + 10)
			break;
		index1 = (index1 + score1 + 1) % state.size();
		index2 = (index2 + score2 + 1) % state.size();
	}

	// print out the last 10 values
	std::cout << "Part 1: next " << state.substr(count, 10) << "\n";
}

// check if the end of the state matches the pattern
bool Match(std::string const &state, std::string const &pattern)
{
	return state.length() >= pattern.length() && state.substr(state.length() - pattern.length()) == pattern;
}

// PART 2
void Part2(std::string const &pattern)
{
	// run until the specified pattern appears
	std::string state = "37";
	int index1 = 0, index2 = 1;
	for (;;)
	{
		int score1 = state[index1] - '0';
		int score2 = state[index2] - '0';
		int sum = score1 + score2;
		if (sum >= 10)
		{
			state.push_back('0' + sum / 10);
			if (Match(state, pattern))
				break;
			state.push_back('0' + sum % 10);
		}
		else
		{
			state.push_back('0' + sum);
		}
		if (Match(state, pattern))
			break;
		index1 = (index1 + score1 + 1) % state.size();
		index2 = (index2 + score2 + 1) % state.size();
	}

	// print the number of entries leading up to the pattern
	std::cout << "Part 1: pattern appears at " << state.length() - pattern.length() << "\n";
}

int main()
{
	// get the input value
	std::string input;
	std::cin >> input;

	Part1(atoi(input.c_str()));
	Part2(input);

	return 0;
}