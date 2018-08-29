#pragma once

#include <utility>
#include <functional>
#include <iterator>
#include <cmath>

namespace BK_sort
{
	template<typename ForwardIterator, typename Compare>
	void make_heap(ForwardIterator begin, ForwardIterator end, Compare comp)
	{
		//iterator type resolution
		_make_heap(begin, end, comp, typename std::iterator_traits<ForwardIterator>::iterator_category());
	}

	template<typename ForwardIterator, typename Compare>
	void _make_heap(ForwardIterator begin, ForwardIterator end, Compare &comp, const std::forward_iterator_tag &tag)
	{
		//if ForwardIterator is not a random access iterator, create a temporary vector
		std::vector<typename ForwardIterator::value_type> temp(begin, end);
		//call the random access overload of _make_heap
		_make_heap(temp.begin(), temp.end(), comp, std::random_access_iterator_tag());
		//copy the temporary vector to the original range
		for (auto it = temp.begin(), end_it = temp.end(); it != end_it; ++it, ++begin)
			*begin = *it;
	}

	template<typename RandomAccessIterator, typename Compare>
	void _make_heap(RandomAccessIterator begin, RandomAccessIterator end, Compare &comp, const std::random_access_iterator_tag &tag)
	{
		assert(begin != end);

		//parent of last element
		auto it = begin + ((static_cast<size_t>(end - begin) - 1) >> 1);
		while (true)
		{
			shift_down(begin, it, end, comp);
			if (it != begin)
				--it;
			else
				break;
		}
	}

	template<typename RandomAccessIterator, typename Compare>
	void shift_down(RandomAccessIterator begin, RandomAccessIterator init_pos, RandomAccessIterator end, Compare &comp)
	{
		using std::swap;
		using size_type = typename std::iterator_traits<RandomAccessIterator>::difference_type;

		//ITERATIVE VERSION
		//the indexes of the root and its children MUST be relative to the beginning of the array being sorted, not the subarray handled by this instance of shift_down
		//while at least the left child exists
		for (size_type root = init_pos - begin, end_pos = end - begin, left_child = (root << 1) | 1, right_child = left_child + 1, to_swap = root;
			left_child < end_pos; 
			left_child = (root << 1) | 1, right_child = left_child + 1)
		{
			if (comp(begin[to_swap], begin[left_child])) //if the current value is smaller than the left child
				to_swap = left_child;
			
			if (right_child < end_pos && comp(begin[to_swap], begin[right_child])) //if the right child exists and is larger than either its parent or the left child
				to_swap = right_child;
			
			if (to_swap == root) //if the parent is larger than both children and we are done
				break;
			else
			{
				swap(begin[to_swap], begin[root]);
				root = to_swap; //otherwise continue to the subtree rooted at to_swap
			}
		}
	}

	template<typename ForwardIterator, typename Compare = std::less<typename std::iterator_traits<ForwardIterator>::value_type>>
	void heap_sort(ForwardIterator begin, ForwardIterator end, Compare comp = Compare())
	{
		//iterator type resolution
		_heap_sort(begin, end, comp, typename std::iterator_traits<ForwardIterator>::iterator_category());
	}

	template<typename ForwardIterator, typename Compare>
	void _heap_sort(ForwardIterator begin, ForwardIterator end, Compare &comp, const std::input_iterator_tag &tag)
	{
		std::vector<typename ForwardIterator::value_type> temp(begin, end);
		_heap_sort(temp.begin(), temp.end(), comp, std::random_access_iterator_tag());
		//copy the temporary vector to the original range
		for (auto it = temp.begin(), end_it = temp.end(); it != end_it; ++it, ++begin)
			*begin = *it;
	}

	template<typename RandomAccessIterator, typename Compare>
	void _heap_sort(RandomAccessIterator begin, RandomAccessIterator end, Compare &comp, const std::random_access_iterator_tag &tag)
	{
		using std::swap;

		BK_sort::make_heap(begin, end, comp);

		end = std::prev(end);
		while (end != begin)
		{
			swap(*begin, *end);
			shift_down(begin, begin, end, comp);
			--end;
		}
	}
}