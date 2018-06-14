#ifndef GAVL_H_
#define GAVL_H_

#include <memory>
#include <functional>
#include <vector>
#include <stack>
#include <queue>

namespace bst {
	template <typename T>
	struct gAVLNode {
		gAVLNode() : _parent(nullptr), _left(nullptr), _right(nullptr), _balance_factor(0) {}
		gAVLNode(std::shared_ptr<gAVLNode> parent, std::shared_ptr<gAVLNode> left, std::shared_ptr<gAVLNode> right) : _parent(parent), _left(left), _right(right), _balance_factor(0) {}

		T _data;

		std::shared_ptr<gAVLNode> _parent;
		std::shared_ptr<gAVLNode> _left;
		std::shared_ptr<gAVLNode> _right;
		int _balance_factor;
	};

	// A self-balancing binary search tree implementing AVL tree -- Space O(n)
	// https://en.wikipedia.org/wiki/AVL_tree
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
			std::shared_ptr<gAVLNode<T>> find(const T& data);
			void retrace_insert(std::shared_ptr<gAVLNode<T>> node);
			void retrace_remove(std::shared_ptr<gAVLNode<T>> node);
			std::shared_ptr<gAVLNode<T>> rotate_right(std::shared_ptr<gAVLNode<T>> A, std::shared_ptr<gAVLNode<T>> B);
			std::shared_ptr<gAVLNode<T>> rotate_left(std::shared_ptr<gAVLNode<T>> A, std::shared_ptr<gAVLNode<T>> B);
			std::shared_ptr<gAVLNode<T>> rotate_right_left(std::shared_ptr<gAVLNode<T>> A, std::shared_ptr<gAVLNode<T>> B);
			std::shared_ptr<gAVLNode<T>> rotate_left_right(std::shared_ptr<gAVLNode<T>> A, std::shared_ptr<gAVLNode<T>> B);

			std::function<int(const T&, const T&)> _comparator;
			std::shared_ptr<gAVLNode<T>> _root;
			std::size_t _size;
	};

	template <typename T>
	gAVL<T>::gAVL(std::function<int(const T&, const T&)> comparator): _comparator(comparator) {
	}

	template <typename T>
	gAVL<T>::~gAVL() {
		// Break parent link of each node in the tree for automatic deallocation when root ref count hits zero
		std::stack<std::shared_ptr<gAVLNode<T>>> s;

		if (_root != nullptr) {
			s.push(_root);
		}

		bool down = true;
		while (!s.empty()) {
			std::shared_ptr<gAVLNode<T>> p = s.top();

			if (down && p->_left != nullptr) {
				s.push(p->_left);
				continue;
			} 

			down = false;
			s.pop();

			// Break link
			p->_parent = nullptr;

			if (p->_right != nullptr) {
				s.push(p->_right);
				down = true;
				continue;
			}
		}
	}

	template <typename T>
	void gAVL<T>::insert(const T& data) {
		std::shared_ptr<gAVLNode<T>> node = std::make_shared<gAVLNode<T>>();
		node->_data = data;
		node->_balance_factor = 0;

		if (_root == nullptr) {
			_root = node;
			_size++;
			return;
		}

		std::shared_ptr<gAVLNode<T>> p = _root;

		while (true) {
			if (_comparator(node->_data, p->_data) < 0) {
				if (p->_left != nullptr) {
					p = p->_left;
				} else {
					p->_left = node;
					node->_parent = p;
					break;
				}
			} else if (_comparator(node->_data, p->_data) == 0) {
				return;
			} else {
				if (p->_right != nullptr) {
					p = p->_right;
				} else {
					p->_right = node;
					node->_parent = p;
					break;
				}
			}
		}

		retrace_insert(node);

		_size++;

		return;
	}

	template <typename T>
	void gAVL<T>::remove(const T& data) {
		if (_root == nullptr) {
			return;
		}

		std::stack<std::pair<std::shared_ptr<gAVLNode<T>>, T>> s;

		T rdata = data;

		while (true) {
			std::shared_ptr<gAVLNode<T>> q = find(rdata);

			if (q == nullptr) {
				return;
			}

			std::shared_ptr<gAVLNode<T>> rep = nullptr;

			if (q->_right == nullptr || q->_left == nullptr) {
				retrace_remove(q);

				if (q->_right == nullptr) {
					rep = q->_left;
				} else {
					rep = q->_right;
				}

				// Remove and potentially replace
				if (q->_parent != nullptr) {
					if (_comparator(q->_data, q->_parent->_data) < 0) {
						q->_parent->_left = rep;
					} else {
						q->_parent->_right = rep;
					}
				} else {
					// This node is the root!
					_root = rep;
				}

				if (rep != nullptr) {
					rep->_parent = q->_parent;	
				}

				// purge, in zero ref fashion (don't risk cycles)
				q->_parent = q->_left = q->_right = nullptr;
				
				break;

			} else {
				// Node to be removed has two children
				// Find minimum node in successor subtree
				std::shared_ptr<gAVLNode<T>> min = q->_right;

				while (min->_left != nullptr) {
					min = min->_left;
				}

				// Replace data later
				rdata = min->_data;
				s.push(std::pair<std::shared_ptr<gAVLNode<T>>, T>(q, rdata));
			}
		}

		while (!s.empty()) {
			std::shared_ptr<gAVLNode<T>> q = s.top().first;
			q->_data = s.top().second;
			s.pop();
		}

		_size--;
	}

	template <typename T> 
	std::size_t gAVL<T>::size() {
		return _size;
	}

	template <typename T>
	int gAVL<T>::height() {
		if (_root == nullptr) {
			return 0;
		}

		std::stack<std::pair<std::shared_ptr<gAVLNode<T>>, int>> s;
		s.push(std::pair<std::shared_ptr<gAVLNode<T>>, int>(_root, 1));

		int max_height = 1;

		bool down = true;
		while (!s.empty()) {
			std::shared_ptr<gAVLNode<T>> p = s.top().first;
			int height = s.top().second;

			if (down && p->_left != nullptr) {
				s.push(std::pair<std::shared_ptr<gAVLNode<T>>, int>(p->_left, height + 1));
				continue;
			}

			down = false;
			s.pop();

			// Break link
			if (height > max_height) {
				max_height = height;
			}

			if (p->_right != nullptr) {
				s.push(std::pair<std::shared_ptr<gAVLNode<T>>, int>(p->_right, height + 1));
				down = true;
				continue;
			}
		}

		return max_height;
	}

	template <typename T>
	std::tuple<int, int> gAVL<T>::height_bounds() {
		static const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
		static const double c   = 1.0 / std::log2(phi);
		static const double b   = (c / 2.0) * std::log2(5) - 2.0;

		double n = static_cast<double>(size());

		return std::tuple<int, int>(
			static_cast<int>(std::floor(std::log2(n + 1.0))),
			static_cast<int>(std::ceil(c * std::log2(n + 2) + b)) - 1
		);
	}

	template <typename T>
	bool gAVL<T>::contains(const T& data) {
		if (find(data) != nullptr) {
			return true;
		}

		return false;
	}

	template <typename T>
	std::vector<T> gAVL<T>::to_stl_vector() {
		std::vector<T> v;
		std::stack<std::shared_ptr<gAVLNode<T>>> s;

		if (_root != nullptr) {
			s.push(_root);
		}

		bool down = true;
		while (!s.empty()) {
			std::shared_ptr<gAVLNode<T>> p = s.top();

			if (down && p->_left != nullptr) {
				s.push(p->_left);
				continue;
			}

			down = false;
			s.pop();

			// push value
			v.push_back(p->_data);

			if (p->_right != nullptr) {
				s.push(p->_right);
				down = true;
			}
		}

		return v;
	}

	template <typename T>
	std::shared_ptr<gAVLNode<T>> gAVL<T>::find(const T& data) {
		std::shared_ptr<gAVLNode<T>> q = _root;

		while (q != nullptr) {
			if (_comparator(data, q->_data) < 0) {
				q = q->_left;
			} else if (_comparator(data, q->_data) == 0) {
				break;
			} else {
				q = q->_right;
			}
		}

		return q;
	}


	/*
		What follows is an adaptation of the pseudocode found at: https://en.wikipedia.org/wiki/AVL_tree
		Or inverted psuedo-code (rotate_right and rotate_left_right are mirrored copies since they were presumed to be self-evident given the other two on the wiki, so the comments might not make sense)
	*/

	template <typename T>
	void gAVL<T>::retrace_insert(std::shared_ptr<gAVLNode<T>> Z) {
		std::shared_ptr<gAVLNode<T>> tmp = Z;
		std::shared_ptr<gAVLNode<T>> N = nullptr;
		std::shared_ptr<gAVLNode<T>> G = nullptr;

		for (std::shared_ptr<gAVLNode<T>> X = Z->_parent; X != nullptr; X = Z->_parent) { // Loop (possibly up to the root)
																						  // balance_factor(X) has to be updated:
			if (Z == X->_right) { // The right subtree increases
				if (X->_balance_factor > 0) { // X is right-heavy
											  // ===> the temporary balance_factor(X) == +2
											  // ===> rebalancing is required.
					G = X->_parent; // Save parent of X around rotations
					if (Z->_balance_factor < 0)      // Right Left Case     (see figure 5)
						N = rotate_right_left(X, Z); // Double rotation: Right(Z) then Left(X)
					else                           // Right Right Case    (see figure 4)
						N = rotate_left(X, Z);     // Single rotation Left(X)
												   // After rotation adapt parent link
				} else {
					if (X->_balance_factor < 0) {
						X->_balance_factor = 0; // Z’s height increase is absorbed at X.
						break; // Leave the loop
					}
					X->_balance_factor = +1;
					Z = X; // Height(Z) increases by 1
					continue;
				}
			} else { // Z == left_child(X): the left subtree increases
				if (X->_balance_factor < 0) { // X is left-heavy
											  // ===> the temporary balance_factor(X) == –2
											  // ===> rebalancing is required.
					G = X->_parent; // Save parent of X around rotations
					if (Z->_balance_factor > 0)      // Left Right Case
						N = rotate_left_right(X, Z); // Double rotation: Left(Z) then Right(X)
					else                           // Left Left Case
						N = rotate_right(X, Z);    // Single rotation Right(X)
												   // After rotation adapt parent link
				} else {
					if (X->_balance_factor > 0) {
						X->_balance_factor = 0; // Z’s height increase is absorbed at X.
						break; // Leave the loop
					}
					X->_balance_factor = -1;
					Z = X; // Height(Z) increases by 1
					continue;
				}
			}
			// After a rotation adapt parent link:
			// N is the new root of the rotated subtree
			// Height does not change: Height(N) == old Height(X)
			N->_parent = G;
			if (G != nullptr) {
				if (X == G->_left)
					G->_left = N;
				else
					G->_right = N;
				break;
			} else {
				_root = N; // N is the new root of the total tree
				break;
			}
			// There is no fall thru, only break; or continue;
		}
		// Unless loop is left via break, the height of the total tree increases by 1.
	}

	template <typename T>
	void gAVL<T>::retrace_remove(std::shared_ptr<gAVLNode<T>> N) {
		std::shared_ptr<gAVLNode<T>> G = nullptr;
		std::shared_ptr<gAVLNode<T>> Z = nullptr;
		int b = 0;

		for (std::shared_ptr<gAVLNode<T>> X = N->_parent; X != nullptr; X = G) { // Loop (possibly up to the root)
			G = X->_parent; // Save parent of X around rotations
						   // BalanceFactor(X) has not yet been updated!
			if (N == X->_left) { // the left subtree decreases
				if (X->_balance_factor > 0) { // X is right-heavy
											// ===> the temporary BalanceFactor(X) == +2
											// ===> rebalancing is required.
					Z = X->_right; // Sibling of N (higher by 2)
					b = Z->_balance_factor;
					if (b < 0)                     // Right Left Case     (see figure 5)
						N = rotate_right_left(X, Z); // Double rotation: Right(Z) then Left(X)
					else                           // Right Right Case    (see figure 4)
						N = rotate_left(X, Z);     // Single rotation Left(X)
												   // After rotation adapt parent link
				} else {
					if (X->_balance_factor == 0) {
						X->_balance_factor = +1; // N’s height decrease is absorbed at X.
						break; // Leave the loop
					}
					N = X;
					N->_balance_factor = 0; // Height(N) decreases by 1
					continue;
				}
			} else { // (N == right_child(X)): The right subtree decreases
				if (X->_balance_factor < 0) { // X is left-heavy
											// ===> the temporary BalanceFactor(X) == –2
											// ===> rebalancing is required.
					Z = X->_left; // Sibling of N (higher by 2)
					b = Z->_balance_factor;
					if (b > 0)                     // Left Right Case
						N = rotate_left_right(X, Z); // Double rotation: Left(Z) then Right(X)
					else                        // Left Left Case
						N = rotate_right(X, Z);    // Single rotation Right(X)
												   // After rotation adapt parent link
				} else {
					if (X->_balance_factor == 0) {
						X->_balance_factor = -1; // N’s height decrease is absorbed at X.
						break; // Leave the loop
					}
					N = X;
					N->_balance_factor = 0; // Height(N) decreases by 1
					continue;
				}
			}
			// After a rotation adapt parent link:
			// N is the new root of the rotated subtree
			N->_parent = G;
			if (G != nullptr) {
				if (X == G->_left)
					G->_left = N;
				else
					G->_right = N;
				if (b == 0)
					break; // Height does not change: Leave the loop
			} else {
				_root = N; // N is the new root of the total tree
			}
			// Height(N) decreases by 1 (== old Height(X)-1)
		}
		// Unless loop is left via break, the height of the total tree decreases by 1.
	}

	template <typename T>
	std::shared_ptr<gAVLNode<T>> gAVL<T>::rotate_right(std::shared_ptr<gAVLNode<T>> X, std::shared_ptr<gAVLNode<T>> Z) {
		// Z is by 2 higher than its sibling
		std::shared_ptr<gAVLNode<T>> t32 = Z->_right; // Inner child of Z
		X->_left = t32;

		if (t32 != nullptr) {
			t32->_parent = X;
		}

		Z->_right = X;
		X->_parent = Z;

		// 1st case, BalanceFactor(Z) == 0, only happens with deletion, not insertion:
		if (Z->_balance_factor == 0) { // t23 has been of same height as t4
			X->_balance_factor = -1;   
			Z->_balance_factor = +1;   
		} else { // 2nd case happens with insertion or deletion:
			X->_balance_factor = 0;
			Z->_balance_factor = 0;
		}

		return Z; // return new root of rotated subtree
	}

	template <typename T>
	std::shared_ptr<gAVLNode<T>> gAVL<T>::rotate_left(std::shared_ptr<gAVLNode<T>> X, std::shared_ptr<gAVLNode<T>> Z) {
		// Z is by 2 higher than its sibling
		std::shared_ptr<gAVLNode<T>> t23 = Z->_left; // Inner child of Z
		X->_right = t23;
		
		if (t23 != nullptr) {
			t23->_parent = X;
		}

		Z->_left = X;
		X->_parent = Z;

		// 1st case, BalanceFactor(Z) == 0, only happens with deletion, not insertion:
		if (Z->_balance_factor == 0) { // t23 has been of same height as t4
			X->_balance_factor = +1;   // t23 now higher
			Z->_balance_factor = -1;   // t4 now lower than X
		} else { // 2nd case happens with insertion or deletion:
			X->_balance_factor = 0;
			Z->_balance_factor = 0;
		}

		return Z; // return new root of rotated subtree
	}

	template <typename T>
	std::shared_ptr<gAVLNode<T>> gAVL<T>::rotate_right_left(std::shared_ptr<gAVLNode<T>> X, std::shared_ptr<gAVLNode<T>> Z) {
		// Z is by 2 higher than its sibling
		std::shared_ptr<gAVLNode<T>> Y = Z->_left; // Inner child of Z
												   // Y is by 1 higher than sibling
		std::shared_ptr<gAVLNode<T>> t3 = Y->_right;
		Z->_left = t3;

		if (t3 != nullptr) {
			t3->_parent = Z;
		}

		Y->_right = Z;
		Z->_parent = Y;
		std::shared_ptr<gAVLNode<T>> t2 = Y->_left;
		X->_right = t2;

		if (t2 != nullptr) {
			t2->_parent = X;
		}

		Y->_left = X;
		X->_parent = Y;

		// 1st case, BalanceFactor(Y) > 0, happens with insertion or deletion:
		if (Y->_balance_factor > 0) { // t3 was higher
			X->_balance_factor = -1;  // t1 now higher
			Z->_balance_factor = 0;
		} else { // 2nd case, BalanceFactor(Y) == 0, only happens with deletion, not insertion:
			if (Y->_balance_factor == 0) {
				X->_balance_factor = 0;
				Z->_balance_factor = 0;
			} else { // 3rd case happens with insertion or deletion:
					 // t2 was higher
				X->_balance_factor = 0;
				Z->_balance_factor = +1;  // t4 now higher
			}
		}
		Y->_balance_factor = 0;

		return Y; // return new root of rotated subtree
	}

	template <typename T>
	std::shared_ptr<gAVLNode<T>> gAVL<T>::rotate_left_right(std::shared_ptr<gAVLNode<T>> X, std::shared_ptr<gAVLNode<T>> Z) {
		// Z is by 2 higher than its sibling
		std::shared_ptr<gAVLNode<T>> Y = Z->_right; // Inner child of Z
												   // Y is by 1 higher than sibling
		std::shared_ptr<gAVLNode<T>> t3 = Y->_left;
		Z->_right = t3;

		if (t3 != nullptr) {
			t3->_parent = Z;
		}

		Y->_left = Z;
		Z->_parent = Y;
		std::shared_ptr<gAVLNode<T>> t2 = Y->_right;
		X->_left = t2;

		if (t2 != nullptr) {
			t2->_parent = X;
		}

		Y->_right = X;
		X->_parent = Y;

		// 1st case, BalanceFactor(Y) > 0, happens with insertion or deletion:
		if (Y->_balance_factor < 0) { 
			X->_balance_factor = +1;  
			Z->_balance_factor = 0;
		} else { // 2nd case, BalanceFactor(Y) == 0, only happens with deletion, not insertion:
			if (Y->_balance_factor == 0) {
				X->_balance_factor = 0;
				Z->_balance_factor = 0;
			} else { // 3rd case happens with insertion or deletion:
				X->_balance_factor = 0;
				Z->_balance_factor = -1;  
			}
		}
		Y->_balance_factor = 0;

		return Y; // return new root of rotated subtree
	}
}


#endif