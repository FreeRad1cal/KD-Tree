#pragma once
#include <iterator>

namespace BK_KD_tree
{
	template<typename Basic_traits>
	struct Tree_Iterator_Traits //Traits needed to make an iterator function as binary search tree iterator
	{
		typedef typename Basic_traits::tree_type tree_type;
		typedef typename Basic_traits::value_type value_type;
		typedef typename const value_type const_value_type;
		typedef typename Basic_traits::node_type::node_pointer node_pointer;
		typedef std::iterator<std::bidirectional_iterator_tag, value_type> iterator_base;
		typedef std::iterator<std::bidirectional_iterator_tag, const_value_type> const_iterator_base;
	};
}