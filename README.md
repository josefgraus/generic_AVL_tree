# Header-only STL-only dependency-free generic AVL tree implementation in C++11(+)

I recently learned to never take for granted self-optimizing data structures written in the language of your choice. So, here's an implementation of a self-balancing binary search tree following [AVL tree](https://en.wikipedia.org/wiki/AVL_tree) in **C++11**. The only file require is `include/gAVL.h`. The rest of the repository is example and example-supporting build toolchain files for Visual Studio 2017.

## Example

The only class provided by the header is ```bst::gAVL```:

```cpp
namespace bst {
	template <typename T>
	class gAVL {
		public:
			gAVL(std::function<int(const T&, const T&)> comparator);
			~gAVL();

			void insert(const T& data);				// Adds the data value to the tree (if it already exists in the tree, does nothing) [O(log(n)]
			void remove(const T& data);				// Removes the data value from the tree (if it does not exist in the tree does nothing) [O(log(n))]
			
			std::size_t size();						// Returns the number of items stored in the tree [O(1)]
			int height();							// Returns the height of the tree (if you *must* know) [O(n)]
			std::tuple<int, int> height_bounds();	// Returns the theoretical upper and lower bounds of the AVL tree (O(1))
			bool contains(const T& data);			// Returns true if the data value is contained in the tree, false otherwise [O(log(n)]

			std::vector<T> to_stl_vector();			// Returns the tree as an ordered vector, with the comparator "least" (negative to all others) value first, and the comparator "most" (positive to all others) last [O(n)]

		protected:
			// None of your business...
	};
...
}
```

The only necessary component that you must supply is an `int` returning **comparator function**, taking `const` references `a`, and `b`. This function allows you to establish an ordered relationship between instanced objects you wish to store in the tree. Given objects `a` and `b`, if they are equal, then the comparator returns `0`. If `a` should come before `b`, then the comparator should return `1` (or any positive `int`). If `a` should come after `b`, then the comparator should return `-1` (or any negative `int`).

See `examples/double_example/double_example.cpp` for an example of **gAVL** over an arbitary number of randomly generated doubles in the interval `[0.0, 100.0]`. Usage with any other datatype should be identical besides the definition of the comparator function.

**Note:** To reverse the order of the sorting in `double_example.cpp`, one simply needs to reverse the sign that the comparator returns, with `+1` for `a < b`, and `-1` for `a > b`.

**double_example.cpp**

```cpp
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
```

**Note:** A Visual Studio 2017 solution file is included in the repository. However, the code should be portable to any other toolchain adequately supporting C++11 (famous last words..)

## Feedback
If you encounter any issues, please open an issue (I am keenly interested in bugs)! Also, while I release this code to the public-ish domain (MIT License), I'd appreciate some feedback as to who found it useful, or who may be forking it for their personal use. It will motivate me to release other packaged work in the future if anyone finds these harried keystrokes useful :)