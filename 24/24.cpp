#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <string.h>

#define DEBUG_PRINT 0

// teams
enum class Team
{
	None = -1,
	ImmuneSystem = 0,
	Infection = 1,
	Count = 2
};

// name of each team
const char * const TeamName[int(Team::Count) + 1] = { "Immune System", "Infection" };

// single damage type
using DamageType = int;

// set of damage types
using DamageTypes = unsigned int;

// map damage type name to damage type index
using DamageTypeMap = std::unordered_map<std::string, int>;
DamageTypeMap damage_types;

// get a damage type from a name
DamageType GetDamageType(const char * &s)
{
	DamageType type = -1;

	char name[32] = { 0 };
	if (sscanf_s(s, "%[^,; ]", name, uint32_t(sizeof(name))) == 1)
	{
		DamageTypeMap::iterator itor = damage_types.find(name);
		if (itor == damage_types.end())
		{
			type = int(damage_types.size());
			damage_types.insert(std::make_pair(name, type));
		}
		else
		{
			type = itor->second;
		}
	}
	s += strlen(name);

	return type;
};

// get a damage type set from a list of names
DamageTypes GetDamageTypes(const char * &s)
{
	DamageTypes types = 0u;

	while (*s != '\0')
	{
		types |= 1u << GetDamageType(s);
		if (*s == ',')
		{
			++s;
			while (isspace(*s))
				++s;
			continue;
		}
		if (*s == ';')
		{
			++s;
			while (isspace(*s))
				++s;
			break;
		}
	}

	return types;
};

// a group of units
struct Group
{
	Team team = Team::None;
	int units = 0;
	int hit_points = 0;
	DamageTypes immune_types = 0;
	DamageTypes weak_types = 0;
	int attack_damage = 0;
	DamageType attack_type = 0;
	int initiative = 0;
};



void ReadGroups(std::vector<Group> &output, std::istream &input)
{
	Team active_team = Team::None;

	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		if (line[0] == '\0')
		{
			// skip blank line
		}
		else if (strncmp(line, TeamName[int(Team::ImmuneSystem)], strlen(TeamName[int(Team::ImmuneSystem)])) == 0)
		{ 
			// start immune system team section
			active_team = Team::ImmuneSystem;
		}
		else if (strncmp(line, TeamName[int(Team::Infection)], strlen(TeamName[int(Team::Infection)])) == 0)
		{
			// start infection team section
			active_team = Team::Infection;
		}
		else
		{
			Group group;

			group.team = active_team;

			// parse the unit description
			char immunities_and_weaknesses[256] = "";
			char attack_type[256] = "";
			if (sscanf_s(line, "%d units each with %d hit points with an attack that does %d %s damage at initiative %d",
					&group.units, &group.hit_points,
					&group.attack_damage, attack_type, uint32_t(sizeof(attack_type)),
					&group.initiative) == 5 ||
				sscanf_s(line, "%d units each with %d hit points (%[^)]) with an attack that does %d %s damage at initiative %d",
					&group.units, &group.hit_points,
					immunities_and_weaknesses, uint32_t(sizeof(immunities_and_weaknesses)),
					&group.attack_damage, attack_type, uint32_t(sizeof(attack_type)),
					&group.initiative) == 6)
			{
				const char token_immune_to[] = "immune to ";
				const char token_weak_to[] = "weak to ";

				// parse immunities and weaknesses
				const char *s = immunities_and_weaknesses;
				while (*s != '\0')
				{
					if (strncmp(s, token_immune_to, sizeof(token_immune_to) - 1) == 0)
					{
						// found immunities
						s += sizeof(token_immune_to) - 1;
						group.immune_types = GetDamageTypes(s);
					}
					else if (strncmp(s, token_weak_to, sizeof(token_weak_to) - 1) == 0)
					{
						// found weaknesses
						s += sizeof(token_weak_to) - 1;
						group.weak_types = GetDamageTypes(s);
					}
					else
					{
						// ?
						break;
					}
				}

				// parse attack type
				s = attack_type;
				group.attack_type = GetDamageType(s);
			}

			output.push_back(group);
		}
	}
}

// get the effective power of a group
int GetEffectivePower(Group const &g)
{
	return g.units * g.attack_damage;
};

// calculate damage group A would apply to group B
int ComputeDamage(Group const &a, Group const &b)
{
	if (b.immune_types & (1 << a.attack_type))
		return 0;
	int damage = a.units * a.attack_damage;
	if (b.weak_types & (1 << a.attack_type))
		damage *= 2;
	return damage;
};


int CountUnitsLeft(std::vector<Group> const &groups, Team team)
{
	int units_left = 0;
	for (Group const &group : groups)
		if (group.team == team)
			units_left += group.units;
	return units_left;
};

Team Simulate(std::vector<Group> &groups)
{
	std::vector<int> group_target_selection_order;
	group_target_selection_order.reserve(groups.size());

	std::vector<int> group_attack_order;
	group_attack_order.reserve(groups.size());

	// compare groups for target selection order
	auto TargetSelectionOrderPredicate = [&](int group_index_a, int group_index_b) -> bool
	{
		Group const &group_a = groups[group_index_a];
		Group const &group_b = groups[group_index_b];

		// compare effective power (higher goes first)
		int a_power = GetEffectivePower(group_a);
		int b_power = GetEffectivePower(group_b);
		if (a_power > b_power)
			return true;
		if (a_power < b_power)
			return false;

		// compare initiative (higher goes first)
		return group_a.initiative > group_b.initiative;
	};

	// compare groups for attack order
	auto AttackOrderPredicate = [&](int group_index_a, int group_index_b) -> bool
	{
		Group const &group_a = groups[group_index_a];
		Group const &group_b = groups[group_index_b];

		// compare initiative (higher goes first)
		return group_a.initiative > group_b.initiative;
	};

#if DEBUG_PRINT
	// track the fight number
	int fight = 0;
#endif

	// no winner yet
	Team winner = Team::None;

	// while the battle has no winner...
	while (winner == Team::None)
	{
#if DEBUG_PRINT
		++fight;
		std::cout << "\nFIGHT #" << fight << "\n";
#endif

		// did the battle make any progress?
		int progress = 0;

		// count live units on each team
		std::array<int, int(Team::Count)> alive;
		std::fill(alive.begin(), alive.end(), 0);
		for (int group_index = 0; group_index < int(groups.size()); ++group_index)
		{
			Group const &group = groups[group_index];
			if (group.units > 0)
			{
#if DEBUG_PRINT
				std::cout << "Group " << group_index << " (" << TeamName[int(group.team)] << ") contains " << group.units << " units\n";
#endif
				alive[int(group.team)] += group.units;
			}
		}


		// TARGET SELECTION PHASE

#if DEBUG_PRINT
		std::cout << "\nTarget Selection\n";
#endif

		// construct the target selection order list
		// (skipping destroyed groups)
		group_target_selection_order.clear();
		for (int group_index = 0; group_index < int(groups.size()); ++group_index)
		{
			Group const &group = groups[group_index];
			if (group.units <= 0)
				continue;
			group_target_selection_order.push_back(group_index);
		}
		std::sort(group_target_selection_order.begin(), group_target_selection_order.end(), TargetSelectionOrderPredicate);

		// groups assigned as targets
		std::vector<int> group_to_attack(groups.size(), -1);
		std::vector<int> group_attacked_by(groups.size(), -1);

		// for each group in target selection order...
		for (int group_index : group_target_selection_order)
		{
			// get the group
			Group const &group = groups[group_index];

#if DEBUG_PRINT
			std::cout << "Group " << group_index << " (" << TeamName[int(group.team)] << ") power=" << GetEffectivePower(group) << " initiative=" << group.initiative << ": ";
#endif

			// find the best target index
			int best_index = -1;
			int best_damage = -1;
			int best_power = -1;
			int best_initiative = -1;
			int target_index = 0;
			for (int target_index : group_target_selection_order)
			{
				Group const &target = groups[target_index];

				// skip teammates
				if (target.team == group.team)
					continue;

#if DEBUG_PRINT >= 2
				std::cout << "\tversus Group " << target_index << ": ";
#endif

				// skip groups already targeted by someone else
				if (group_attacked_by[target_index] >= 0)
				{
#if DEBUG_PRINT >= 2
					std::cout << "already targeted by Group " << group_attacked_by[target_index] << "\n";
#endif
					continue;
				}

				// get attack properties for the target
				const int target_damage = ComputeDamage(group, target);

				const int target_power = GetEffectivePower(target);
				const int target_initiative = target.initiative;
#if DEBUG_PRINT >= 2
				std::cout << "damage=" << target_damage << " power=" << target_power << " initiative=" << target_initiative << "\n";
#endif

				// update the best target group
				if (target_damage > best_damage ||
					(target_damage == best_damage &&
					(target_power > best_power ||
					(target_power == best_power &&
					(target_initiative > best_initiative)))))
				{
					best_index = target_index;
					best_damage = target_damage;
					best_power = target_power;
					best_initiative = target_initiative;
				}
			}

#if DEBUG_PRINT >= 2
			std::cout << "\n\t";
#endif

			if (best_index < 0)
			{
				// could not find a target
#if DEBUG_PRINT
				std::cout << "has no one to attack\n";
#endif
			}
			else if (group_attacked_by[best_index] >= 0)
			{
				// target is blocked
#if DEBUG_PRINT
				std::cout << "was blocked by Group " << group_attacked_by[best_index] << "\n";
#endif
			}
			else if (best_damage <= 0)
			{
				// target is immune
#if DEBUG_PRINT
				std::cout << "cannot damage Group " << best_index << "\n";
#endif
			}
			else
			{
				// engage the target
#if DEBUG_PRINT
				std::cout << "targets Group " << best_index << " for " << best_damage << " damage\n";
#endif
				group_to_attack[group_index] = best_index;
				group_attacked_by[best_index] = group_index;
			}
		}


		// ATTACK PHASE
#if DEBUG_PRINT
		std::cout << "\nAttack\n";
#endif

		// construct the attack order list
		// (skipping destroyed groups and groups with no target)
		group_attack_order.clear();
		for (int group_index = 0; group_index < int(groups.size()); ++group_index)
		{
			Group const &group = groups[group_index];
			if (group.units <= 0)
				continue;
			if (group_to_attack[group_index] < 0)
				continue;
			group_attack_order.push_back(group_index);
		}
		std::sort(group_attack_order.begin(), group_attack_order.end(), AttackOrderPredicate);

		// for each group in attack order...
		for (int group_index : group_attack_order)
		{
			// get the acting group
			Group const &group = groups[group_index];

			// skip the group if it got destroyed before its action
			if (group.units <= 0)
				continue;

			// get the target group
			int target_index = group_to_attack[group_index];
			Group &target = groups[target_index];

			// apply damage to the target group
			const int damage = ComputeDamage(group, target);
			const int kills = std::min(target.units, damage / target.hit_points);
			target.units -= kills;
			progress += kills;
#if DEBUG_PRINT
			std::cout << "Group " << group_index << " (" << TeamName[int(group.team)] << ") initiative=" << group.initiative << " attacks Group " << target_index << " for " << damage << " damage, killing " << kills << " units (" << target.units << " left)\n";
#endif

			// if the target's team has no survivors...
			alive[int(target.team)] -= kills;
			if (alive[int(target.team)] == 0)
			{
				// our team wins
				winner = group.team;
			}
		}

		// abort if no progress was made (stalemate)
		if (!progress)
			break;
	}

	// report the winning team
	return winner;
}

// PART 1
void Part1(std::vector<Group> const &initial_groups)
{
	// start with initial groups
	std::vector<Group> groups = initial_groups;

	// simulate the battle
	Team winner = Simulate(groups);

	// print the result
	int units_left = CountUnitsLeft(groups, winner);
	std::cout << "Part 1: " << TeamName[int(winner)] << " wins with " << units_left << " units left\n";
}

// PART 2
void Part2(std::vector<Group> const &initial_groups)
{
	std::vector<Group> groups;

	// amount of boost applied
	int boost = 0;

	// simulate battles until the immune system gets a win
	for(;;)
	{
		// increment the boost
		++boost;

		// start with initial groups
		groups = initial_groups;

		// apply boost to immune system groups
		for (Group &group : groups)
			if (group.team == Team::ImmuneSystem)
				group.attack_damage += boost;

		// simulate the battle
		Team winner = Simulate(groups);

		std::cout << "Part 2: with " << boost << " boost, ";
		if (winner == Team::None)
		{
			std::cout << "nobody wins\n";
		}
		else
		{
			// print the result
			int units_left = CountUnitsLeft(groups, winner);
			std::cout << TeamName[int(winner)] << " wins with " << units_left << " units left\n";

			// we're done if the immune system wins
			if (winner == Team::ImmuneSystem)
				break;
		}
	}
}

int main()
{
	std::vector<Group> initial_groups;
	ReadGroups(initial_groups, std::cin);

	Part1(initial_groups);
	Part2(initial_groups);

	return 0;
}