#include <iostream>
#include <vector>
#include <list>

// https://adventofcode.com/2018/day/9

void Part(int partNumber, int playerCount, int lastMarbleWorth)
{
	// track score for each player
	std::vector<int64_t> playerScores(playerCount, 0);

	// start with the first player
	// (index is player number minus 1)
	int currentPlayerIndex = 0;

	// the (current) winning player
	int winningPlayerIndex = -1;
	int64_t winningPlayerScore = -1;

	// the circle of marbles;
	// use a linked list instead of a vector because its insert and erase operations 
	// are O(1) instead of O(N), making the overall game O(N) instead of O(N^2)
	std::list<int> marbleCircle(1, 0);
	std::list<int>::iterator currentMarble = marbleCircle.begin();

	// for each marble to play...
	for (int nextMarbleWorth = 1; nextMarbleWorth <= lastMarbleWorth; ++nextMarbleWorth)
	{
		// if the marble's value is evenly divisible by 23...
		if (nextMarbleWorth % 23 == 0)
		{
			int64_t &currentPlayerScore = playerScores[currentPlayerIndex];

			// score the marble that would have been played
			currentPlayerScore += nextMarbleWorth;

			// get the  marble 7 places counter-clockwise from the current marble
			// (and we have to do this because list iterators are not random access)
			for (int i = 0; i < 7; ++i)
			{
				if (currentMarble == marbleCircle.begin())
					currentMarble = marbleCircle.end();
				--currentMarble;
			}

			// score the marble
			currentPlayerScore += *currentMarble;

			// remove it from the circle and make its clockwise neighbor current
			currentMarble = marbleCircle.erase(currentMarble);

			// update the winning player
			if (winningPlayerScore < currentPlayerScore)
			{
				winningPlayerScore = currentPlayerScore;
				winningPlayerIndex = currentPlayerIndex;
			}
		}
		else
		{
			// insert the marble between the 1st and 2nd marble clockwise
			// from the current marble and make it the new current marble
			++currentMarble;
			if (currentMarble == marbleCircle.end())
				currentMarble = marbleCircle.begin();
			++currentMarble;
			currentMarble = marbleCircle.insert(currentMarble, nextMarbleWorth);
		}

		// move to the next player
		currentPlayerIndex = (currentPlayerIndex + 1) % playerCount;
	}

	std::cout << "Part " << partNumber << ": player " << winningPlayerIndex + 1 << " wins with " << winningPlayerScore << " points\n";
}

int main()
{
	// get the player count and what the last marble is worth
	int playerCount  = 0, lastMarbleWorth = 0;
	char line[256];
	std::cin.getline(line, sizeof(line));
	sscanf_s(line, "%d players; last marble is worth %d points", &playerCount, &lastMarbleWorth);

	Part(1, playerCount, lastMarbleWorth);
	Part(2, playerCount, lastMarbleWorth * 100);

	return 0;
}