#pragma once
#include "tuple.h"
#include <cassert>
#include <iostream>

namespace BK_KD_tree
{
	template<typename Traits>
	class KD_tree_node
	{
	public:
		typedef typename Traits::value_type	value_type;
		typedef KD_tree_node				node_type;
		typedef node_type*					node_pointer;
		typedef typename Traits::size_type	size_type;

		//the dimension of the coordinate system
		static constexpr size_type dimension = value_type::first_type::dimension();

		template<typename Value>
		KD_tree_node(Value &&value, node_pointer left_child_ptr = nullptr, node_pointer right_child_ptr = nullptr) : val(std::forward<Value>(value)), left(left_child_ptr), right(right_child_ptr) {}
		KD_tree_node(const KD_tree_node &node) : val(node.val), left(nullptr), right(nullptr) {}

		value_type& value() { return val; }
		const value_type& value() const { return val; }

		//template<size_type index>
		//auto coordinate() ->decltype(get<index>(val.first)) { return get<index>(val.first); }

		node_pointer& left_child() { return left; }
		const node_pointer& left_child() const { return left; }

		node_pointer& right_child() { return right; }
		const node_pointer& right_child() const { return right; }

	private:
		value_type		val;
		node_pointer	left;
		node_pointer	right;
	};

	template<typename Traits>
	void swap(KD_tree_node<Traits> &a, KD_tree_node<Traits> &b)
	{
		KD_tree_node<Traits> *a_left = a.left_child(), *a_right = a.right_child();
		a.left_child() = b.left_child();
		a.right_child() = b.right_child();
		b.left_child() = a_left;
		b.right_child() = a_right;
	}
}