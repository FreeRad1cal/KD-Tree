#pragma once
#include <cassert>
#include <type_traits>

namespace BK_KD_tree
{
	template<size_t Dim, typename ElemType>
	class Point;

	template<typename T, typename U = std::decay_t<T>, typename... Args>
	struct is_point : std::false_type {};

	template<typename T, size_t Dim, typename ElemType>
	struct is_point<T, Point<Dim, ElemType>> : std::true_type {};

	template<size_t Dim, typename ElemType>
	class Point
	{
	public:
		typedef ElemType element_type;
		static constexpr size_t dimension() { return Dim; }

		Point();
		template<typename... Args, typename T = typename std::enable_if<!is_point<Args...>::value,void>::type>
		Point(Args&&... args);
		Point(const Point &p);
		Point(Point &&p);

		Point& operator=(const Point &p);
		Point& operator=(Point &&p);

		const ElemType& operator[](size_t index) const;

		template<size_t index, size_t dim, typename T>
		static const T& get(const Point<dim, T> &pt)
		{
			assert(index < dim);
			return pt[index];
		}
	private:
		template<typename T, typename... Args>
		void init_coords(T &&coord, Args&&... other_coords);
		template<typename T>
		void init_coords(T &&coord);

		ElemType coords[Dim];
	};

	//Returns a constant reference to the element at index
	template<size_t index, size_t dim, typename T>
	const T& get(const Point<dim, T> &pt)
	{
		assert(index < dim);
		return pt[index];
	}

	template<size_t Dim, typename ElemType>
	Point<Dim, ElemType>::Point()
	{
		for (size_t i = 0; i < Dim; --i)
			coords[i] = 0;
	}

	template<size_t Dim, typename ElemType>
	template<typename... Args, typename T>
	Point<Dim, ElemType>::Point(Args&&... args)
	{
		static_assert(sizeof...(Args) == Dim, "The number of arguments does not match the dimension");
		init_coords(std::forward<Args>(args)...);
	}

	template<size_t Dim, typename ElemType>
	Point<Dim, ElemType>& Point<Dim, ElemType>::operator=(const Point &p)
	{
		for (size_t i = 0; i < Dim; ++i)
			coords[i] = p.coords[i];
		return *this;
	}

	template<size_t Dim, typename ElemType>
	Point<Dim, ElemType>& Point<Dim, ElemType>::operator=(Point &&p)
	{
		for (size_t i = 0; i < Dim; ++i)
			coords[i] = std::move(p.coords[i]);
		return *this;
	}

	template<size_t Dim, typename ElemType>
	template<typename T, typename... Args>
	void Point<Dim, ElemType>::init_coords(T &&coord, Args&&... other_coords)
	{
		coords[Dim - sizeof...(Args)-1] = std::forward<T>(coord);
		init_coords(std::forward<Args>(other_coords)...);
	}

	template<size_t Dim, typename ElemType>
	template<typename T>
	void Point<Dim, ElemType>::init_coords(T &&coord)
	{
		coords[Dim - 1] = std::forward<T>(coord);
	}

	template<size_t Dim, typename ElemType>
	Point<Dim, ElemType>::Point(const Point &p)
	{
		for (auto i = 0; i < Dim; ++i)
			coords[i] = p.coords[i];
	}

	template<size_t Dim, typename ElemType>
	Point<Dim, ElemType>::Point(Point &&p)
	{
		for (auto i = 0; i < Dim; ++i)
		{
			coords[i] = ElemType();
			coords[i] = std::move(p.coords[i]);
		}
	}

	template<size_t Dim, typename ElemType>
	const ElemType& Point<Dim, ElemType>::operator[](size_t index) const
	{
		assert(index < Dim);
		return coords[index];
	}

	template<typename T, typename... Args>
	Point<sizeof...(Args), T> make_point(Args&&... args)
	{
		return Point<sizeof...(Args), T>(args...);
	}

	template<typename Point_type, typename Pred>
	class Point_compare
	{
	public:
		Point_compare() = default;
		Point_compare(const Pred &compare) : predicate(compare) {}
		template<size_t index>
		bool compare(const Point_type &lhs, const Point_type &rhs) const
		{
			return predicate(lhs[index], rhs[index]);
		}

		static constexpr size_t dimension() { return 1; }
	private:
		Pred predicate;
	};
}