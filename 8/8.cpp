#include <iostream>
#include <vector>

// https://adventofcode.com/2018/day/8

// tree node
struct Node
{
	std::vector<std::unique_ptr<Node>> childNodes;
	std::vector<int> metadataEntries;
};

// read in the tree nodes
void ReadTree(Node &output, std::istream &input)
{
	// get number of child nodes
	int numChildNodes;
	input >> numChildNodes;
	output.childNodes.resize(numChildNodes);

	// get number of metadata entries
	int numMetadataEntries;
	input >> numMetadataEntries;
	output.metadataEntries.resize(numMetadataEntries);

	// recursively read in the child nodes
	for (std::unique_ptr<Node> &childNode : output.childNodes)
	{ 
		childNode = std::make_unique<Node>();
		ReadTree(*childNode, input);
	}

	// read in the metadata entries
	for (int &metadataEntry : output.metadataEntries)
	{
		input >> metadataEntry;
	}
}

// recursively sum the metadata entries for a node and all it children
int GetSumOfMetadataEntries(Node const &node)
{
	int sum = 0;

	// add the metadata entries for this node
	for (int metadataEntry : node.metadataEntries)
	{
		sum += metadataEntry;
	}

	// add the sum from each child node
	for (std::unique_ptr<Node> const &childNode : node.childNodes)
	{
		sum += GetSumOfMetadataEntries(*childNode);
	}

	return sum;
}

// PART 1
void Part1(Node const &root)
{
	int sum = GetSumOfMetadataEntries(root);
	std::cout << "Part 1: sum of metadata entries is " << sum << "\n";
}

// get the value of a node
int GetNodeValue(Node const &node)
{
	int value = 0;

	if (node.childNodes.size() == 0)
	{
		// node has no children:
		// its value is the sum of its metadata entries
		for (int metadataEntry : node.metadataEntries)
		{
			value += metadataEntry;
		}
	}
	else
	{
		// node has children:
		// its value is the sum of the values of the child nodes indexed by the metadata entries
		for (int metadataEntry : node.metadataEntries)
		{
			if (metadataEntry > 0 && metadataEntry <= node.childNodes.size())
			{
				value += GetNodeValue(*node.childNodes[metadataEntry - 1]);
			}
		}
	}

	return value;
}

// PART 2
void Part2(Node const &root)
{
	int value = GetNodeValue(root);
	std::cout << "Part 2: value of the root node is " << value << "\n";
}

int main()
{
	// read in the tree nodes
	Node root;
	ReadTree(root, std::cin);

	Part1(root);
	Part2(root);

	return 0;
}