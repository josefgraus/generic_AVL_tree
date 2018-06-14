#include <limits>
#include <random>
#include <iostream>
#include <cassert>

#include <gAVL.h>

using namespace bst;

// The comparator function 
// If a and b are equal, return 0
// If a should come before b, return -1 (or any negative int)
// If a should come after b, return 1 (or any positive int)
std::function<int(const double&, const double&)> comparator = [](const double& a, const double& b) -> int {
	if (std::fabs(a - b) < std::numeric_limits<double>::epsilon()) {
		return 0;
	} else if (a < b) {
		return -1;
	} 

	return 1;
};

int main(void) {
	gAVL<double> tree(comparator);

	/*** CHANGE FOR FUN (2^k random doubles will be generated for this test) ***/
	// I'm not responsible for stdout buffer flooding
	int k = 4;

	// Random number gen
	std::random_device rd;  
	std::mt19937 gen(rd()); 
	std::uniform_real_distribution<> dis(0.0, 100.0);
	int n = static_cast<int>(std::pow(2, k)) - 1;

	// Fill the tree with n random numbers
	std::vector<double> u; 
	std::cout << "Random doubles in [0.0, 100.0]:" << std::endl;
	for (int i = 0; i < n; ++i) {
		double d = dis(gen);
		tree.insert(d);
		u.push_back(d);

		assert(tree.contains(d));

		std::cout << d << "   ";
	}

	std::cout << std::endl << std::endl;

	// Show how the tree sorted the numbers upon insertion
	std::cout << "In-order traversal, left to right:" << std::endl;
	std::vector<double> v = tree.to_stl_vector();
	for (auto d : v) {
		std::cout << d << "   ";
	}
	std::cout << std::endl;

	int height = tree.height();
	auto height_bounds = tree.height_bounds();

	// Show respect for theoretical upper and lower tree height bounds
	std::cout << " Bound Height: " << std::get<1>(tree.height_bounds()) << std::endl;
	std::cout << "Actual Height: " << tree.height() << std::endl;
	std::cout << "         Size: " << tree.size() << std::endl << std::endl;

	assert(std::get<0>(height_bounds) <= height && height <= std::get<1>(height_bounds));

	// Remove the first 50% of doubles
	std::cout << "Remove first 1/2 -- New In-order traversal, left to right: " << std::endl;
	int m = static_cast<int>(std::floor(static_cast<double>(n) * 0.50));

	for (int i = 0; i < m; ++i) {
		tree.remove(u[i]);
	}

	// Show new ordering
	v = tree.to_stl_vector();
	for (auto d : v) {
		std::cout << d << "   ";
	}
	std::cout << std::endl;

	// Again, respect for upper/lower bounds
	std::cout << "   Max Height: " << std::get<1>(tree.height_bounds()) << std::endl;
	std::cout << "Actual Height: " << tree.height() << std::endl;
	std::cout << "         Size: " << tree.size() << std::endl << std::endl;

	assert(std::get<0>(height_bounds) <= height && height <= std::get<1>(height_bounds));

	// Re-add the entire set of doubles randomly generated previously
	std::cout << "Re-add the entire set -- New In-order traversal, left to right: " << std::endl;
	std::cout << "*Note that tree does not insert values that already exist in the tree*" << std::endl;

	for (int i = 0; i < n; ++i) {
		tree.insert(u[i]);
	}

	// Show order -- should be identical to the first time
	v = tree.to_stl_vector();
	for (auto d : v) {
		std::cout << d << "   ";
	}
	std::cout << std::endl;

	// More upper/lower bounds assurances
	std::cout << "   Max Height: " << std::get<1>(tree.height_bounds()) << std::endl;
	std::cout << "Actual Height: " << tree.height() << std::endl;
	std::cout << "         Size: " << tree.size() << std::endl << std::endl;

	assert(std::get<0>(height_bounds) <= height && height <= std::get<1>(height_bounds));

	system("PAUSE");

	return 0;
}