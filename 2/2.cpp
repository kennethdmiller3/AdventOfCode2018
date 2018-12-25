#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// https://adventofcode.com/2018/day/2

// read a list of identifiers from the specified input stream
void ReadIdentifiers(std::vector<std::string> &output, std::istream &input)
{
	std::string id;
	while (input >> id)
	{
		output.push_back(id);
	}
}

// PART 1
void Part1(std::vector<std::string> const &identifiers)
{
	int numWith2 = 0, numWith3 = 0;
	for (std::string const &identifier : identifiers)
	{
		int counts[26] = { 0 };
		int has2 = 0, has3 = 0;
		for (char const letter : identifier)
		{
			if (isalpha(letter))
			{
				int &count = counts[tolower(letter) - 'a'];
				if (count == 2)
				{
					--has2;
				}
				else if (count == 3)
				{
					--has3;
				}
				++count;
				if (count == 2)
				{
					++has2;
				}
				else if (count == 3)
				{
					++has3;
				}
			}
		}
		if (has2 != 0)
		{
			++numWith2;
		}
		if (has3 != 0)
		{
			++numWith3;
		}
	}
	int checksum = numWith2 * numWith3;
	std::cout << "Part 1: checksum " << checksum << "\n";
}

// PART 2
void Part2(std::vector<std::string> const &identifiers)
{
	for (int index2 = 1; index2 < identifiers.size(); ++index2)
	{
		std::string const &identifier2 = identifiers[index2];

		for (int index1 = 0; index1 < index2; ++index1)
		{
			std::string const &identifier1 = identifiers[index1];

			if (identifier1.size() == identifier2.size())
			{
				int mismatches = 0;
				int position = -1;
				for (int pos = 0; pos < identifier1.size() && mismatches <= 1; ++pos)
				{
					if (identifier1[pos] != identifier2[pos])
					{
						++mismatches;
						position = pos;
					}
				}

				if (mismatches == 1)
				{
					std::cout << "Part 2: identifiers\n\t" << identifier1 << "\n\t" << identifier2 << "\ndiffer at position " << position << "\n";
					std::cout << "common letters " << identifier1.substr(0, position) << identifier1.substr(position + 1, identifier1.size() - position - 1);
					return;
				}
			}
		}
	}
}

int main()
{
	std::vector<std::string> identifiers;
	ReadIdentifiers(identifiers, std::cin);

	Part1(identifiers);
	Part2(identifiers);

	return 0;
}