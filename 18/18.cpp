#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

// https://adventofcode.com/2018/day/18


//#define DEBUG_PRINT

using History = std::unordered_map<std::string, uint32_t>;

// run one update
void Update(std::string &output, std::string const &input, uint16_t width, uint16_t height)
{
	// sweep through the input state
	for (uint8_t y = 0; y < height; ++y)
	{
		const uint8_t ym1 = y > 0 ? y - 1 : 0;
		const uint8_t yp1 = y < height - 1 ? y + 1 : height - 1;

		for (uint8_t x = 0; x < width - 1; ++x)
		{
			const uint8_t xm1 = x > 0 ? x - 1 : 0;
			const uint8_t xp1 = x < width - 2 ? x + 1 : width - 2;

			// count neighbors
			char center = 0;
			uint8_t open_count = 0, tree_count = 0, yard_count = 0;
			for (uint8_t yy = ym1; yy <= yp1; ++yy)
			{
				for (uint8_t xx = xm1; xx <= xp1; ++xx)
				{
					const char c = input[yy * width + xx];
					if (xx == x && yy == y)
					{
						center = c;
					}
					else
					{
						switch (c)
						{
						case '.': ++open_count; break;
						case '|': ++tree_count; break;
						case '#': ++yard_count; break;
						}
					}
				}
			}

			// get the new cell value
			switch (center)
			{
			case '.': if (tree_count >= 3) center = '|'; break;
			case '|': if (yard_count >= 3) center = '#'; break;
			case '#': if (tree_count < 1 || yard_count < 1) center = '.'; break;
			}
			output[y * width + x] = center;
		}
	}
}

void PrintResultForPart(int part, std::string const &state)
{
	uint16_t tree_total = 0, yard_total = 0;
	for (const char c : state)
	{
		switch (c)
		{
		case '|': ++tree_total; break;
		case '#': ++yard_total; break;
		}
	}
	std::cout << "Part " << part << ": " << tree_total << " trees * " << yard_total << " yards = " << tree_total * yard_total << "\n";
}

// PART 1
void Part1(std::string const &initial, uint8_t width, uint8_t height)
{
	// set current state to initial state
	std::string state = initial, next = initial;

	for (uint8_t step = 1; step <= 10; ++step)
	{
		Update(next, state, width, height);
		std::swap(state, next);

#ifdef DEBUG_PRINT
		std::cout << "step " << step << ":\n" << state << '\n';
#endif
	}

	// print result
	PrintResultForPart(1, state);
}

// PART 2
void Part2(std::string const &initial, uint8_t width, uint8_t height)
{
	// set current state to initial state
	std::string state = initial, next = initial;

	// history of all previous states
	std::vector<std::string> history;
	history.push_back(state);

	// map to look up the step number for a given state
	std::unordered_map <std::string, uint32_t> lookup;

#ifdef DEBUG_PRINT
	std::cout << "initial:\n" << initial << '\n';
#endif

	// for each step...
	const uint32_t step_count = 1000000000;
	for (uint32_t step = 1; step <= step_count; ++step)
	{
		// update the state
		Update(next, state, width, height);
		std::swap(state, next);

		// look for a previous matching state
		History::iterator itor = lookup.find(state);
		if (itor != lookup.end())
		{
			// find the repeat period
			const uint32_t period = step - itor->second;
			std::cout << "Part 2: step " << step << " repeats step " << itor->second << " (period " << period << ")\n";

			// jump ahead to the final step
			step = step_count;

			// retrieve the state from the history that would match the final state
			const uint32_t folded_step = (step - itor->second) % period + itor->second;
			std::cout << "Part 2: step " << step << " equivalent to step " << folded_step << "\n";
			state = history[folded_step];
		}
		else
		{
			// add state to history
			history.push_back(state);
			lookup[state] = step;
		}

#ifdef DEBUG_PRINT
		std::cout << state << '\n';
#endif
	}

	// print result
	PrintResultForPart(2, state);
}

int main()
{
	// read in the initial state
	// (new trick: read the entire file into a string stream)
	std::string initial;
	{
		std::stringstream stream;
		stream << std::cin.rdbuf();
		initial = std::move(stream.str());
	}
	uint8_t width = uint8_t(initial.find_first_of('\n') + 1);
	uint8_t height = uint8_t(initial.size() / width);

#ifdef DEBUG_PRINT
	std::cout << "initial:\n" << initial << '\n';
#endif

	Part1(initial, width, height);
	Part2(initial, width, height);

	return 0;
}