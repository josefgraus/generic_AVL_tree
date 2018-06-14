#include <limits>
#include <random>
#include <iostream>
#include <cassert>

#include <gAVL.h>

using namespace bst;

std::function<bool(const double&, const double&)> less_than = [](const double& a, const double& b) -> bool {
	return a < b;
};

std::function<bool(const double&, const double&)> equal = [](const double& a, const double& b) -> bool {
	return (std::fabs(a - b) < std::numeric_limits<double>::epsilon());
};

int main(void) {
	gAVL<double> tree(less_than, equal);

	int k = 4;

	std::random_device rd;  
	std::mt19937 gen(rd()); 
	std::uniform_real_distribution<> dis(0.0, 100.0);
	int n = static_cast<int>(std::pow(2, k)) - 1;

	//std::vector<double> u; // = { 30.0, 20.0, 25.0, 10, 5, 8, 2, 7, 12 };
	std::vector<double> u = { 72.8802,   43.6026,   36.4165,   47.7681,   31.8813,   10.4903,   34.1303,   63.9099,  86.9684,   83.8471,   9.92424,   53.796,   98.1777,   68.4051,   63.4467 };
	n = static_cast<int>(u.size());
	std::cout << "Random doubles in [0.0, 100.0]:" << std::endl;
	/*for (int i = 0; i < n; ++i) {
		double d = dis(gen);
		tree.insert(d);
		u.push_back(d);

		assert(tree.contains(d));

		std::cout << d << "   ";
	}*/
	for (auto d : u) {
		std::cout << d << "   ";
		tree.insert(d);
		assert(tree.contains(d));
	}

	std::cout << std::endl << std::endl;

	std::cout << "In-order traversal, left to right:" << std::endl;
	std::vector<double> v = tree.to_stl_vector();
	for (auto d : v) {
		std::cout << d << "   ";
	}
	std::cout << std::endl;

	int height = tree.height();
	auto height_bounds = tree.height_bounds();

	std::cout << " Bound Height: " << std::get<1>(tree.height_bounds()) << std::endl;
	std::cout << "Actual Height: " << tree.height() << std::endl;
	std::cout << "         Size: " << tree.size() << std::endl << std::endl;

	tree.debug_print();

	assert(std::get<0>(height_bounds) <= height && height <= std::get<1>(height_bounds));

	std::cout << "Remove first random 3/4 -- New In-order traversal, left to right: " << std::endl;
	int m = static_cast<int>(std::floor(static_cast<double>(n) * 0.50));

	for (int i = 0; i < m; ++i) {
		tree.remove(u[i]);
	}

	v = tree.to_stl_vector();
	for (auto d : v) {
		std::cout << d << "   ";
	}
	std::cout << std::endl;


	std::cout << "   Max Height: " << std::get<1>(tree.height_bounds()) << std::endl;
	std::cout << "Actual Height: " << tree.height() << std::endl;
	std::cout << "         Size: " << tree.size() << std::endl << std::endl;

	tree.debug_print();

	system("PAUSE");

	return 0;
}