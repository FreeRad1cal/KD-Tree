#pragma once
#include <vector>
#include <iterator>
#include <iostream>
#include <queue>
#include <cassert>
#include <stdexcept>
#include "heap_sort.h"

namespace BK_heap
{
	template<typename T, typename Container = std::vector<T>, typename Compare = std::less<typename Container::value_type>>
	class Priority_queue
	{
		friend Priority_queue operator+(const Priority_queue&, const Priority_queue&);
		friend Priority_queue operator+(Priority_queue&&, Priority_queue&&);
	public:
		typedef typename Container::value_type	value_type;
		typedef Compare							compare_type;
		typedef	typename Container::size_type	size_type;

		explicit Priority_queue	(const Compare &comp = Compare()) : end_pos(0), c(comp) {}
		Priority_queue			(const Priority_queue &h) : arr(h.arr), end_pos(h.end_pos), c(h.c) {}
		Priority_queue			(Priority_queue &&h) : arr(std::move(h.arr)), end_pos(h.end_pos), c(std::move(h.c)) {}
		template<typename InputIterator>
		Priority_queue			(InputIterator begin, InputIterator end, const Compare &comp = Compare());

		Priority_queue& operator=(const Priority_queue &h) { arr = h.arr; end_pos = h.end_pos; c = h.c;  return *this; }
		Priority_queue& operator=(Priority_queue &&h) { arr = std::move(h.arr); end_pos = h.end_pos; c = std::move(h.c); return *this; }

		const T&	top		() const { assert(!empty()); return arr.front(); }
		template<typename U>
		void		push	(U &&val);
		void		pop		();
		template<typename U>
		void		replace	(U &&val);
		size_type	size	() const { return arr.size(); }
		bool		empty	() const { return arr.empty(); }
		void		erase	(const value_type &val);
		void		clear	() { arr.clear(); end_pos = 0; }
		void		debug	(std::ostream &out);

	protected:
		Container arr;
		size_type end_pos;
		Compare c;

		size_type		parent(size_type pos);
		size_type		left_child(size_type pos);
		size_type		right_child(size_type pos);
		size_type		sibling(size_type pos);
		size_type		largest_child(size_type pos);
		size_type		shift_up(size_type pos);
		void			shift_down(size_type pos);
		bool			find(size_type &pos, const value_type &val);
	};

	template<typename T, typename Container, typename Compare>
	Priority_queue<T, Container, Compare> operator+(const Priority_queue<T, Container, Compare> &lhs, const Priority_queue<T, Container, Compare> &rhs)
	{
		Priority_queue<T, Container, Compare> ret(lhs);
		for (auto it = rhs.arr.begin(), end_it = rhs.arr.end(); it != end_it; ++it)
			ret.push(*it);

		return std::move(ret);
	}

	template<typename T, typename Container, typename Compare>
	Priority_queue<T, Container, Compare> operator+(Priority_queue<T, Container, Compare> &&lhs, Priority_queue<T, Container, Compare> &&rhs)
	{
		Priority_queue<T, Container, Compare> ret(std::move(lhs));

		Priority_queue<T, Container, Compare> rhs_copy(rhs);
		for (auto it = rhs.arr.begin(), end_it = rhs.arr.end(); it != end_it; ++it)
			ret.push(std::move(*it));

		return std::move(ret);
	}

	template<typename T, typename Container, typename Compare>
	void Priority_queue<T, Container, Compare>::debug(std::ostream &out)
	{
		if (empty())
			return;

		std::queue<size_type> q;
		q.push(0);

		out << "--------------" << std::endl;

		while (!q.empty())
		{
			size_type cur = q.front();
			out << arr[cur] << std::endl;
			q.pop();

			if (left_child(cur) < end_pos)
				q.push(left_child(cur));
			if (right_child(cur) < end_pos)
				q.push(right_child(cur));
		}

		out << "--------------" << std::endl;
	}

	template<typename T, typename Container, typename Compare>
	template<typename InputIterator>
	Priority_queue<T, Container, Compare>::Priority_queue(InputIterator begin, InputIterator end, const Compare &comp)
		:	arr(begin, end),
			end_pos(arr.size()),
			c(comp)
	{
		BK_sort::make_heap(arr.begin(), arr.end(), comp);
	}

	template<typename T, typename Container, typename Compare>
	template<typename U>
	void 
	Priority_queue<T, Container, Compare>::push(U &&val)
	{
		arr.emplace_back(std::forward<U>(val));
		shift_up(end_pos++);
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type 
	Priority_queue<T, Container, Compare>::parent(size_type pos)
	{
		if (pos == 0)
			throw std::invalid_argument("No parent");

		//if the index is odd
		if ((pos & 1) != 0)
			return (pos - 1) >> 1; //(pos - 1)/2
		else
			return (pos >> 1) - 1; //(pos - 2)/2
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type 
	Priority_queue<T, Container, Compare>::left_child(size_type pos)
	{
		return (pos << 1) | 1;
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type
	Priority_queue<T, Container, Compare>::right_child(size_type pos)
	{
		return (pos << 1) + 2;
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type
	Priority_queue<T, Container, Compare>::largest_child(size_type pos)
	{
		size_type lc = left_child(pos), rc = lc + 1;
		if (rc < end_pos)
			return arr[lc] < arr[rc] ? rc : lc;
		else if (lc < end_pos)
			return lc;
		else
			throw std::invalid_argument("No left child");
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type
	Priority_queue<T, Container, Compare>::sibling(size_type pos)
	{
		if ((pos & 1) != 0) // if left child of parent
			return right_child((pos - 1) >> 1);
		else //if right child of parent
			return left_child((pos >> 1) - 1);
	}

	template<typename T, typename Container, typename Compare>
	typename Priority_queue<T, Container, Compare>::size_type
	Priority_queue<T, Container, Compare>::shift_up(size_type pos)
	{
		assert(pos < end_pos);

		using std::swap;

		if (pos == 0 || c(arr[pos], arr[parent(pos)]))
		{
			//nothing to do, the heap is valid
			return pos;
		}
		else
		{
			swap(arr[pos], arr[parent(pos)]);
			return shift_up(parent(pos));
		}
	}

	template<typename T, typename Container, typename Compare>
	void
	Priority_queue<T, Container, Compare>::shift_down(size_type pos)
	{
		using std::swap;

		size_type largest;
		try //if the left child exists
		{
			//find the largest child
			largest = largest_child(pos);
		}
		catch(...) //otherwise, the bottom of the heap has been reached and we are done
		{
			return;
		}

		//if the current node is smaller than the largest child
		if (c(arr[pos], arr[largest]))
		{
			//swap and continue rebalancing
			swap(arr[pos], arr[largest]);
			shift_down(largest);
		}
		//otherwise the heap is valid and we are done
	}

	template<typename T, typename Container, typename Compare>
	void Priority_queue<T, Container, Compare>::pop()
	{
		using std::swap;
		swap(arr.front(), arr.back());
		arr.pop_back();
		--end_pos;
		shift_down(0);
	}

	template<typename T, typename Container, typename Compare>
	template<typename U>
	void 
	Priority_queue<T, Container, Compare>::replace(U &&val)
	{
		arr.front() = std::forward<U>(val);
		shift_down(0);
	}

	template<typename T, typename Container, typename Compare>
	bool
	Priority_queue<T, Container, Compare>::find(size_type &pos, const value_type &val)
	{
		if (pos < end_pos)
		{
			if (c(arr[pos], val))
				return false;
			else if (c(val, arr[pos]))
			{
				size_type old_pos = pos;
				return find(pos = left_child(old_pos), val) || find(pos = right_child(old_pos), val);
			}
			else
				return true;
		}
		else
			return false;
	}

	template<typename T, typename Container, typename Compare>
	void 
	Priority_queue<T, Container, Compare>::erase(const value_type &val)
	{
		using std::swap;

		size_type loc = 0;
		if (find(loc, val))
		{
			swap(arr[loc], arr.back());
			arr.pop_back();
			--end_pos;
			shift_down(loc);
		}
	}

	template class Priority_queue<int>;
}