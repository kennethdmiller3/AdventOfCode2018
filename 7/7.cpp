#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <assert.h>

// https://adventofcode.com/2018/day/7

// a constraint; constraint.first must be completed before constraint.second
typedef std::pair<char, char> Constraint;

// read in the constraints
void ReadConstraints(std::vector<Constraint> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		std::pair<char, char> constraint;
		if (sscanf_s(line, "Step %c must be finished before step %c can begin.", &constraint.first, 1, &constraint.second, 1) == 2)
			output.push_back(constraint);
	}
}

// PART 1
void Part1(std::vector<Constraint> const &constraints)
{
	std::string order;

	// convert the array of constraints into an array of bit masks
	// representing what steps become available upon completing that step
	unsigned int stepsMadeAvailable[26] = { 0U };
	unsigned int stepsPossible = 0U;
	for (Constraint const &constraint : constraints)
	{
		stepsMadeAvailable[constraint.first - 'A'] |= 1 << (constraint.second - 'A');
		stepsPossible |= 1 << (constraint.first - 'A');
		stepsPossible |= 1 << (constraint.second - 'A');
	}

	// keep track of steps already completed
	unsigned int stepsCompleted = 0U;

	while (stepsCompleted != stepsPossible)
	{
		// determine which steps are available
		unsigned int stepsAvailable = stepsPossible & ~stepsCompleted;
		for (int i = 0; i < 26; ++i)
		{
			// if a step is not completed, steps that depend on it are not available
			if ((stepsCompleted & (1 << i)) == 0)
			{
				stepsAvailable &= ~stepsMadeAvailable[i];
			}
		}

		// choose the lowest available step
		for (int i = 0; i < 26 && stepsAvailable != 0; ++i)
		{
			if ((stepsAvailable & (1 << i)) != 0)
			{
				// move the step from available to completed
				stepsAvailable &= ~(1 << i);
				stepsCompleted |= (1 << i);

				// add to the ordered list of steps
				order.push_back(i + 'A');
				break;
			}
		}
	}

	std::cout << "Part 1: instruction order " << order << "\n";
}

struct Worker
{
	int time = 0;
	int task = -1;
};

bool operator<(Worker const &a, Worker const &b)
{
	if (a.time > b.time)
		return true;
	if (a.time < b.time)
		return false;
	if (a.task > b.task)
		return true;
	return false;
}

// PART 2
void Part2(std::vector<Constraint> const &constraints)
{
	// convert the array of constraints into an array of bit masks
	// representing what steps become available upon completing that step
	unsigned int stepsMadeAvailable[26] = { 0U };
	unsigned int stepsPossible = 0U;
	for (Constraint const &constraint : constraints)
	{
		stepsMadeAvailable[constraint.first - 'A'] |= 1 << (constraint.second - 'A');
		stepsPossible |= 1 << (constraint.first - 'A');
		stepsPossible |= 1 << (constraint.second - 'A');
	}

	// worker pool
	const int maxWorkers = 5;
	std::vector<Worker> activeWorkers;
	int completedTime = 0;

	// keep track of steps in progress
	unsigned int stepsPending = 0U;

	// keep track of steps already completed
	unsigned int stepsCompleted = 0U;

	while (stepsCompleted != stepsPossible)
	{
		// determine which steps are available
		unsigned int stepsAvailable = stepsPossible & ~stepsCompleted & ~stepsPending;
		for (int i = 0; i < 26; ++i)
		{
			// if a step is not completed, steps that depend on it are not available
			if ((stepsCompleted & (1 << i)) == 0)
			{
				stepsAvailable &= ~stepsMadeAvailable[i];
			}
		}

		// assign tasks to idle workers
		for (int i = 0; i < 26 && stepsAvailable != 0 && activeWorkers.size() < maxWorkers; ++i)
		{
			if ((stepsAvailable & (1 << i)) != 0)
			{
				// move the step from available to pending
				stepsAvailable &= ~(1 << i);
				stepsPending |= (1 << i);

				// add the worker
				Worker worker;
				worker.task = i;
				worker.time = completedTime + (61 + i);
				activeWorkers.push_back(worker);
				std::push_heap(activeWorkers.begin(), activeWorkers.end());
			}
		}
		
		assert(!activeWorkers.empty());

		// get the first worker to finish their task
		std::pop_heap(activeWorkers.begin(), activeWorkers.end());
		Worker const &worker = activeWorkers.back();

		// save the time their task was completed
		completedTime = worker.time;

		// complete their assigned task
		stepsPending &= ~(1 << worker.task);
		stepsCompleted |= (1 << worker.task);

		// free up the worker
		activeWorkers.pop_back();
	}

	std::cout << "Part 2: time to complete is " << completedTime << "\n";
}

int main()
{
	// read in the constraints
	std::vector<Constraint> constraints;
	ReadConstraints(constraints, std::cin);

	Part1(constraints);
	Part2(constraints);

	return 0;
}