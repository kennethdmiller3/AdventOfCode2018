#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

// https://adventofcode.com/2018/day/17

struct Point
{
	uint16_t x;
	uint16_t y;
};

struct Rect
{
	uint16_t x0;
	uint16_t y0;
	uint16_t x1;
	uint16_t y1;
};

// read the rects
void ReadRects(std::vector<Rect> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Rect rect;
		if (sscanf_s(line, "x=%hu, y=%hu..%hu", &rect.x0, &rect.y0, &rect.y1) == 3)
		{
			rect.x1 = rect.x0;
			output.push_back(rect);
		}
		else if (sscanf_s(line, "y=%hu, x=%hu..%hu", &rect.y0, &rect.x0, &rect.x1) == 3)
		{
			rect.y1 = rect.y0;
			output.push_back(rect);
		}
	}
}

Rect GetBounds(std::vector<Rect> &rects)
{
	Rect bounds = { UINT16_MAX, UINT16_MAX, 0, 0 };
	for (Rect const &rect : rects)
	{
		if (bounds.x0 > rect.x0)
			bounds.x0 = rect.x0;
		if (bounds.y0 > rect.y0)
			bounds.y0 = rect.y0;
		if (bounds.x1 < rect.x1)
			bounds.x1 = rect.x1;
		if (bounds.y1 < rect.y1)
			bounds.y1 = rect.y1;
	}
	return bounds;
}

void BuildLayout(std::string &layout, uint16_t &width, uint16_t &height, std::vector<Rect> const &rects, Rect const &bounds)
{
	width = bounds.x1 - bounds.x0 + 1U + 1U;
	height = bounds.y1 - bounds.y0 + 1U;
	layout.clear();
	layout.resize(width * height, '.');
	for (uint16_t y = 0; y < height; ++y)
	{
		layout[y * width + width - 1U] = '\n';
	}
	for (Rect const &rect : rects)
	{
		for (uint16_t y = rect.y0; y <= rect.y1; ++y)
		{
			for (uint16_t x = rect.x0; x <= rect.x1; ++x)
			{
				layout[y * width + x] = '#';
			}
		}
	}
}

int main()
{
	// read the rects
	std::vector<Rect> rects;
	ReadRects(rects, std::cin);

	// determine the bounds of the rects
	Rect bounds = GetBounds(rects);

	// extend bounds to the left and right
	--bounds.x0;
	++bounds.x1;

	// shift rects by the upper left corner of the bounds
	for (Rect &rect : rects)
	{
		rect.x0 -= bounds.x0;
		rect.y0 -= bounds.y0;
		rect.x1 -= bounds.x0;
		rect.y1 -= bounds.y0;
	}

	// build the layout
	std::string layout;
	uint16_t width = 0, height = 0;
	BuildLayout(layout, width, height, rects, bounds);

	// add the source flow
	Point source = { 500U - bounds.x0, 0U };
	std::vector<Point> flows;
	flows.push_back(source);

	// water counts
	uint32_t flow_count = 0;
	uint32_t rest_count = 0;

	// while there are active flows...
	while (!flows.empty())
	{
		Point flow = flows.back();
		flows.pop_back();

		switch (layout[flow.y * width + flow.x])
		{
		case '.':
			// flow into unobstructed space
			++flow_count;
			layout[flow.y * width + flow.x] = '|';

			if (flow.y < height - 1U)
			{
				// continue downwards
				++flow.y;
				flows.push_back(flow);
			}
			break;

		case '|':
			// stop at existing flowing water
			break;

		case '#':
		case '~':
			// hit obstruction
			if (flow.y > 0)
			{
				--flow.y;
				if (layout[flow.y * width + flow.x] == '|')
				{
					// convert flow above to spread
					layout[flow.y * width + flow.x] = 'V';
					flows.push_back(flow);
				}
			}
			break;

		case 'V':
			// spread laterally
			{
				// convert back to flow
				layout[flow.y * width + flow.x] = '|';

				// flow to the left
				uint16_t scan_left = flow.x;
				bool wall_left = false;
				bool stop_left = false;
				do
				{
					--scan_left;
					switch (layout[flow.y * width + scan_left])
					{
					case '#':
						// wall to the left
						wall_left = true;
						stop_left = true;
						break;
					case '.':
						// empty space to the left
						++flow_count;
						// fall through!
					case 'V':
						layout[flow.y * width + scan_left] = '|';
						// fall through!
					case '|':
						// flow to the left
						switch (layout[(flow.y + 1U) * width + scan_left])
						{
						case '.':
							// flow down
							flows.push_back({ scan_left, flow.y + 1U });
							stop_left = true;
							break;
						case '|':
						case 'V':
							// join flow below
							stop_left = true;
							break;
						}
						break;
					default:
						break;
					}
					if (scan_left == 0)
						stop_left = true;
				} while (!stop_left);

				// flow to the right
				uint16_t scan_right = flow.x;
				bool wall_right = false;
				bool stop_right = false;
				do
				{
					++scan_right;
					switch (layout[flow.y * width + scan_right])
					{
					case '#':
						// wall to the right
						wall_right = true;
						stop_right = true;
						break;
					case '.':
						// empty space to the right
						++flow_count;
						// fall through!
					case 'V':
						layout[flow.y * width + scan_right] = '|';
						// fall through!
					case '|':
						// flow to the right
						switch (layout[(flow.y + 1U) * width + scan_right])
						{
						case '.':
							// flow down
							flows.push_back({ scan_right, flow.y + 1U });
							stop_right = true;
							break;
						case '|':
						case 'V':
							// join flow below
							stop_right = true;
							break;
						}
						break;
					default:
						break;
					}
					if (scan_right == width - 1)
						stop_right = true;
				} while (!stop_right);

				if (wall_left && wall_right)
				{
					// convert to water at rest
					for (uint16_t x = scan_left + 1; x <= scan_right - 1U; ++x)
					{
						assert(layout[flow.y * width + x] == '|' || layout[flow.y * width + x] == 'V');
						--flow_count;
						++rest_count;
						layout[flow.y * width + x] = '~';
					}

					if (flow.y > 0)
					{
						--flow.y;
						for (uint16_t x = scan_left + 1; x <= scan_right - 1U; ++x)
						{
							if (layout[flow.y * width + x] == '|')
							{
								// convert flow above to spread
								layout[flow.y * width + x] = 'V';
								flows.push_back(flow);
							}
						}
					}
				}
			}
			break;
		}
	}

	std::cout << "Part 1: water count " << flow_count + rest_count << "\n";
	std::cout << "Part 2: rest count " << rest_count << "\n";

	return 0;
}
