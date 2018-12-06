#include <iostream>
#include <string>
#include <algorithm>

// https://adventofcode.com/2018/day/5

// read in the polymer sequence
void ReadPolymer(std::string &output, std::istream &input)
{
	input >> output;
}

// PART 1
void Part1(std::string const &polymer)
{
	std::string result = polymer;
	
	// work backwards to minimize the number of characters that need to get moved
	for (size_t index = result.length() - 1; index > 0; --index)
	{
		// remove adjacent units if they have opposite polarity;
		// this effectively moves the index up by two
		if (abs(result[index] - result[index - 1]) == 'a' - 'A')
		{
			result.erase(index - 1, 2);
		}
	}

	// output the result
	std::cout << "Part 1: " << result.length() << " units remain\n";
}


// PART 2
void Part2(std::string const &polymer)
{
	// best result so far
	size_t bestLength = SIZE_MAX;
	char bestUnit = 0;

	for (char testUnit = 'A'; testUnit <= 'Z'; ++testUnit)
	{
		std::cout << "testing " << testUnit << "/" << char(tolower(testUnit)) << "...";

		// removing all instances of the test unit regardless of polarity
		std::string result;
		result.reserve(polymer.length());
		for (char const unit : polymer)
		{
			if (toupper(unit) != testUnit)
				result.push_back(unit);
		}

		// work backwards to minimize the number of characters that need to get moved
		for (size_t index = result.length() - 1; index > 0; --index)
		{
			// remove adjacent units if they have opposite polarity;
			// this effectively moves the index up by two
			if (abs(result[index] - result[index - 1]) == 'a' - 'A')
			{
				result.erase(index - 1, 2);
			}
		}

		// units left after reaction
		size_t testLength = result.length();

		std::cout << testLength << " units remain\n";

		// update the best length if the test length is shorter
		if (bestLength > testLength)
		{
			bestLength = testLength;
			bestUnit = testUnit;
		}
	}

	// output the best result
	std::cout << "Part 2: " << bestLength << " units remain after removing all " << bestUnit << "/" << char(tolower(bestUnit)) << "\n";
}

int main()
{
	// read in the polymer sequence
	std::string polymer;
	ReadPolymer(polymer, std::cin);

	Part1(polymer);
	Part2(polymer);

	return 0;
}
