#include <iostream>
#include <string>
#include <vector>

// https://adventofcode.com/2018/day/3

struct Rect
{
	int i;
	int x;
	int y;
	int w;
	int h;
};

// read a list of claim identifiers and coverage rectangles
void ReadClaims(std::vector<Rect> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Rect rect = { 0 };
		sscanf_s(line, "#%d @ %d,%d: %dx%d", &rect.i, &rect.x, &rect.y, &rect.w, &rect.h);
		output.push_back(rect);
	}
}

// PART 1
void Part1(std::vector<Rect> const &claims)
{
	int conflict = 0;

	const int W = 1000;
	const int H = 1000;
	std::vector<unsigned char> claimcount(H * W, 0);

	// for each claim...
	for (int i = 0; i < claims.size(); ++i)
	{
		// get the claim rectangle
		Rect const &claim = claims[i];

		// for each square inch of the claimed rectangle...
		for (int y = claim.y; y < claim.y + claim.h; ++y)
		{
			for (int x = claim.x; x < claim.x + claim.w; ++x)
			{
				// if this is the second claim...
				if (++claimcount[y * W + x] == 2)
				{
					// mark the square inch as a conflict
					++conflict;
				}
			}
		}
	}

	std::cout << "Part 1: " << conflict << " square inches conflict\n";
}

// PART 2
void Part2(std::vector<Rect> const &claims)
{
	const int W = 1000;
	const int H = 1000;
	std::vector<int> claimant(H * W, -1);
	std::vector<bool> conflicted(claims.size(), false);

	// for each claim...
	for (int i = 0; i < claims.size(); ++i)
	{
		// get the claim rectangle
		Rect const &claim = claims[i];

		// for each square inch of the claimed rectangle...
		for (int y = claim.y; y < claim.y + claim.h; ++y)
		{
			for (int x = claim.x; x < claim.x + claim.w; ++x)
			{
				if (claimant[y * W + x] < 0)
				{
					// first to claim
					claimant[y * W + x] = i;
				}
				else
				{
					conflicted[claimant[y * W + x]] = true;
					conflicted[i] = true;
				}
			}
		}
	}

	for (int i = 0; i < conflicted.size(); ++i)
	{
		if (!conflicted[i])
		{
			std::cout << "Part 2: claim " << claims[i].i << " has no conflict\n";
		}
	}
}

int main()
{
	std::vector<Rect> claims;
	ReadClaims(claims, std::cin);

	Part1(claims);
	Part2(claims);

	return 0;
}