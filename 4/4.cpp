#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <assert.h>

// https://adventofcode.com/2018/day/4

enum class EventType : unsigned char
{
	Undefined,
	BeginsShift,
	FallsAsleep,
	WakesUp
};

// guard event record
struct EventRecord
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	EventType event;
	unsigned short id;
};

// comparison operator for sorting records chronologically
bool operator<(EventRecord const &a, EventRecord const &b)
{
	if (a.year < b.year)
		return true;
	if (a.year > b.year)
		return false;
	if (a.month < b.month)
		return true;
	if (a.month > b.month)
		return false;
	if (a.day < b.day)
		return true;
	if (a.day > b.day)
		return false;
	if (a.hour < b.hour)
		return true;
	if (a.hour > b.hour)
		return false;
	if (a.minute < b.minute)
		return true;
	if (a.minute > b.minute)
		return false;
	if (int(a.event) < int(b.event))
		return true;
	if (int(a.event) > int(b.event))
		return false;
	if (int(a.id) < int(b.id))
		return true;
	return false;
}

// read a list of guard event ecords
void ReadRecords(std::vector<EventRecord> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		EventRecord record = { 0 };

		// parse the time stamp and get the event description
		char event[256] = "";
		sscanf_s(line, "[%hu-%hhu-%hhu %hhu:%hhu] %[^\r\n]s", &record.year, &record.month, &record.day, &record.hour, &record.minute, event, unsigned int(sizeof(event)));

		// get event type based on event description
		if (sscanf_s(event, "Guard #%hd begins shift", &record.id) == 1)
		{
			record.event = EventType::BeginsShift;
		}
		else if (strcmp(event, "falls asleep") == 0)
		{
			record.event = EventType::FallsAsleep;
		}
		else if (strcmp(event, "wakes up") == 0)
		{
			record.event = EventType::WakesUp;
		}

		output.push_back(record);
	}
}

void Part1(std::vector<EventRecord> const &records)
{
	// total sleep time in minutes per guard
	std::unordered_map<unsigned short, unsigned int> durationPerGuardId;

	// information about the best guard
	int bestGuardId = -1;
	unsigned int bestGuardSleepDuration = 0;

	// information about the current guard
	int currentGuardId = -1;
	unsigned int currentGuardSleepDuration = 0;
	unsigned char currentGuardSleepStart = 0;

	for (EventRecord const &record : records)
	{
		switch (record.event)
		{
		case EventType::BeginsShift:
			// finish previous guard event sequence
			if (currentGuardId >= 0)
			{
				// write duration back
				durationPerGuardId[currentGuardId] = currentGuardSleepDuration;

				// update the best guard if the current sleep duration is better
				if (bestGuardSleepDuration < currentGuardSleepDuration)
				{
					bestGuardSleepDuration = currentGuardSleepDuration;
					bestGuardId = currentGuardId;
				}
			}

			// start new guard event sequence
			currentGuardId = record.id;
			currentGuardSleepDuration = durationPerGuardId[currentGuardId];
			break;

		case EventType::FallsAsleep:
			// save when current guard fell asleep
			assert(record.hour == 0);
			currentGuardSleepStart = record.minute;
			break;

		case EventType::WakesUp:
			// accumulate sleep time for the current guard
			assert(record.hour == 0);
			currentGuardSleepDuration += record.minute - currentGuardSleepStart;
			break;
		}

		if (currentGuardId >= 0)
		{
			// write duration back
			durationPerGuardId[currentGuardId] = currentGuardSleepDuration;

			// update the best guard if the current sleep duration is better
			if (bestGuardSleepDuration < currentGuardSleepDuration)
			{
				bestGuardSleepDuration = currentGuardSleepDuration;
				bestGuardId = currentGuardId;
			}
		}
	}

	// how many times the best guard was asleep on a given minute
	std::vector<unsigned short> countPerMinute(60, 0);

	// information about the best minute
	int bestMinute = -1;
	unsigned short bestCount = 0;

	// information about the current guard
	currentGuardId = -1;

	for (EventRecord const &record : records)
	{
		switch (record.event)
		{
		case EventType::BeginsShift:
			// start new guard event sequence
			currentGuardId = record.id;
			break;

		case EventType::FallsAsleep:
			if (currentGuardId == bestGuardId)
			{
				// save when the best guard fell asleep
				assert(record.hour == 0);
				currentGuardSleepStart = record.minute;
			}
			break;

		case EventType::WakesUp:
			if (currentGuardId == bestGuardId)
			{
				// accumulate sleep counts per minute
				assert(record.hour == 0);
				for (unsigned char currentMinute = currentGuardSleepStart; currentMinute < record.minute; ++currentMinute)
				{
					++countPerMinute[currentMinute];

					// update the best minute if the current minute is better
					if (bestCount < countPerMinute[currentMinute])
					{
						bestCount = countPerMinute[currentMinute];
						bestMinute = currentMinute;
					}
				}
			}
			break;
		}
	}

	// print the result
	std::cout << "Part 1: guard with most minutes asleep is " << bestGuardId << " (" << bestGuardSleepDuration << " minutes)\n";
	std::cout << "\tmost commonly asleep at minute " << bestMinute << " (" << bestCount << " times)\n";
	std::cout << "\tid * minute = " << bestGuardId * bestMinute << "\n";
}


void Part2(std::vector<EventRecord> const &records)
{
	std::unordered_map<unsigned short, std::vector<int>> countPerMinutePerId;

	int bestGuardId = -1;
	int bestMinute = -1;
	unsigned short bestCount = 0;

	int currentGuardId = -1;
	unsigned char currentGuardSleepStart = 0;

	for (EventRecord const &record : records)
	{
		switch (record.event)
		{
		case EventType::BeginsShift:
			// start new guard event sequence
			currentGuardId = record.id;
			break;

		case EventType::FallsAsleep:
			// save when the current guard fell asleep
			assert(record.hour == 0);
			currentGuardSleepStart = record.minute;
			break;

		case EventType::WakesUp:
			assert(record.hour == 0);
			// accumulate sleep counts per minute
			std::vector<int> &countPerMinute = countPerMinutePerId[currentGuardId];
			countPerMinute.resize(60);
			for (unsigned char currentMinute = currentGuardSleepStart; currentMinute < record.minute; ++currentMinute)
			{
				++countPerMinute[currentMinute];

				// update the best guard and best minute if the current minute is better
				if (bestCount < countPerMinute[currentMinute])
				{
					bestGuardId = currentGuardId;
					bestCount = countPerMinute[currentMinute];
					bestMinute = currentMinute;
				}
			}
			break;
		}
	}

	// print the result
	std::cout << "Part 2: guard most commonly asleep is " << bestGuardId << " (" << bestCount << " times at minute " << bestMinute << ")\n";
	std::cout << "\tid * minute = " << bestGuardId * bestMinute << "\n";
}

int main()
{
	// read in the records
	std::vector<EventRecord> records;
	ReadRecords(records, std::cin);

	// sort the records chronologically
	std::sort(records.begin(), records.end());

	Part1(records);
	Part2(records);

	return 0;
}