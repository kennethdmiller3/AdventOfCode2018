#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

// the order that carts update is based on current location in "reading order"
// first row left to right, second row left to right, etc.

// when a cart reaches an intersection,
// 1st time: turn left
// 2nd time: go straight
// 3rd time: turn right
// then repeats the pattern

enum class Direction : char
{
	Up = 0, Right = 1, Down = 2, Left = 3, Count = 4
};

enum class Turn : char
{
	Left = 0, Straight = 1, Right = 2, Count = 3
};

struct Position
{
	short x = 0;
	short y = 0;
};

bool operator<(Position const &a, Position const &b)
{
	if (a.y < b.y)
		return true;
	if (a.y > b.y)
		return false;
	if (a.x < b.x)
		return true;
	return false;
}

struct Cart
{
	Position pos;	// current position
	Direction dir;	// current direction
	char under;		// what's under the cart
	Turn turn;		// next turn direction
	char id;
};

bool operator<(Cart const &a, Cart const &b)
{
	return a.pos < b.pos;
}

void Simulate(std::string const &layout, short width, short height, int part)
{
	// cart locations
	short x = 0, y = 0;
	char id = 0;
	std::vector<Cart> carts;
	for (char const c : layout)
	{
		switch (c)
		{
		case '^': carts.push_back({ {x, y}, Direction::Up,    '|', Turn::Left, id++ }); break;
		case '>': carts.push_back({ {x, y}, Direction::Right, '-', Turn::Left, id++ }); break;
		case 'v': carts.push_back({ {x, y}, Direction::Down,  '|', Turn::Left, id++ }); break;
		case '<': carts.push_back({ {x, y}, Direction::Left,  '-', Turn::Left, id++ }); break;
		case '\n': x = -1; ++y; break;
		}

		++x;
	}

	const char image[int(Direction::Count)] = { '^', '>', 'v', '<' };

	std::string state = layout;

	for (int tick = 0; ; ++tick)
	{
		// perform one "tick"
		for (int i = 0; i < carts.size(); ++i)
		{
			Cart &cart = carts[i];

			// restore the track
			state[cart.pos.y * width + cart.pos.x] = cart.under;

			// get the new position
			switch (cart.dir)
			{
			case Direction::Up:		--cart.pos.y; break;
			case Direction::Right:	++cart.pos.x; break;
			case Direction::Down:	++cart.pos.y; break;
			case Direction::Left:	--cart.pos.x; break;
			}
			assert(cart.pos.x >= 0 && cart.pos.x < width && cart.pos.y >= 0 && cart.pos.y < height);

			// update based on what's at the new position
			switch (state[cart.pos.y * width + cart.pos.x])
			{
			case '^':
			case '>':
			case 'v':
			case '<':
				// collided with another cart
				std::cout << "Part " << part << ": crash at " << cart.pos.x << ", " << cart.pos.y << " on tick " << tick << "\n";
				state[cart.pos.y * width + cart.pos.x] = 'X';

				if (part == 1)
				{
					// for part 1, stop on the first collision
					return;
				}

				// remove the colliding carts
				for (int j = 0; j < carts.size(); ++j)
				{
					const Cart &cart2 = carts[j];
					if (cart.pos.x == cart2.pos.x &&
						cart.pos.y == cart2.pos.y &&
						i != j)
					{
						state[cart2.pos.y * width + cart2.pos.x] = cart2.under;
						if (i > j)
							--i;
						carts.erase(carts.begin() + j);
						break;
					}
				}
				carts.erase(carts.begin() + i);
				--i;

				continue;

			case '/':
				// track curve
				switch (cart.dir)
				{
				case Direction::Up:		cart.dir = Direction::Right; break;
				case Direction::Right:	cart.dir = Direction::Up; break;
				case Direction::Down:	cart.dir = Direction::Left; break;
				case Direction::Left:	cart.dir = Direction::Down; break;
				}
				break;

			case '\\':
				// track curve
				switch (cart.dir)
				{
				case Direction::Up:		cart.dir = Direction::Left; break;
				case Direction::Right:	cart.dir = Direction::Down; break;
				case Direction::Down:	cart.dir = Direction::Right; break;
				case Direction::Left:	cart.dir = Direction::Up; break;
				}
				break;

			case '+':
				// intersection
				switch (cart.turn)
				{
				case Turn::Left:
					cart.dir = Direction((int(cart.dir) + int(Direction::Count) - 1) % int(Direction::Count));
					break;
				case Turn::Right:
					cart.dir = Direction((int(cart.dir) + 1) % int(Direction::Count));
					break;
				}
				cart.turn = Turn((int(cart.turn) + 1) % int(Turn::Count));
			}

			// save the track
			cart.under = state[cart.pos.y * width + cart.pos.x];

			// place the cart
			state[cart.pos.y * width + cart.pos.x] = image[int(cart.dir)];
		}

		// for part 2, stop when only one cart remains
		if (part == 2 && carts.size() == 1)
		{
			std::cout << "Part " << part << ": last cart at " << carts.back().pos.x << ", " << carts.back().pos.y << "\n";
			return;
		}

		// sort the carts for the next tick
		std::sort(carts.begin(), carts.end());
	}
}

int main()
{
	// read the initial layout
	std::string layout;
	std::string line;
	short width = 0, height = 0;
	while (std::getline(std::cin, line))
	{
		layout.append(line);
		layout.push_back('\n');

		width = std::max(width, short(line.size() + 1));
		++height;
	}

	Simulate(layout, width, height, 1);
	Simulate(layout, width, height, 2);

	return 0;
}