#pragma once

#include <exception>
#include <type_traits>
#include <utility>
#include <iostream>

namespace BK_Tuple
{
	template<size_t index, typename T>
	struct Element_type;

	template<typename... Args>
	class Tuple;

	template<typename T, typename U = std::decay_t<T>>
	struct is_tuple : std::false_type {};

	template<typename T, typename... Args>
	struct is_tuple<T, Tuple<Args...>> : std::true_type {};

	template<typename T, typename... Args>
	class Tuple<T, Args...>
	{
	public:
		Tuple() : elem(), next() {}
		Tuple(const T &arg, const Args&... args) : elem(arg), next(args...) {}
		Tuple(T &&arg, Args&&... args) : elem(std::move(arg)), next(std::move(args)...) {}
		Tuple(const Tuple &tup) : elem(tup.elem), next(tup.next) {}
		Tuple(Tuple &&tup);

		Tuple& operator=(const Tuple &tup);
		Tuple& operator=(Tuple &&tup);

		static constexpr size_t dimension() { return 1 + Tuple<Args...>::dimension(); }

		template<size_t index, typename... _Args>
		static const typename Element_type<index, Tuple<_Args...>>::type& get(const Tuple<_Args...> &tup);
	private:
		T elem;
		Tuple<Args...> next;
	};

	template<typename T, typename ...Args>
	Tuple<T, Args...>::Tuple(Tuple &&tup) : elem(), next(std::move(tup.next))
	{
		using std::swap;
		swap(elem, tup.elem);
	}

	template<typename T, typename... Args>
	Tuple<T, Args...>& Tuple<T, Args...>::operator=(const Tuple &tup)
	{
		if (this != &tup)
		{
			elem = tup.elem;
			next = tup.next;
		}

		return *this;
	}

	template<typename T, typename... Args>
	Tuple<T, Args...>& Tuple<T, Args...>::operator=(Tuple &&tup)
	{
		using std::swap;

		if (this != &tup)
		{
			swap(elem, tup.elem);
			next = std::move(tup.next);
		}

		return *this;
	}

	template<typename T>
	class Tuple<T>
	{
	public:
		Tuple() : elem() {}
		Tuple(const T &arg) : elem(arg) {}
		Tuple(T &&arg) : elem(std::move(arg)) {}
		Tuple(const Tuple &tup) : elem(tup.elem) {}
		Tuple(Tuple &&tup);

		Tuple& operator=(const Tuple &tup);
		Tuple& operator=(Tuple &&tup);

		static constexpr size_t dimension() { return 1; }

		template<size_t index, typename _T>
		static const typename Element_type<index, Tuple<_T>>::type& get(const Tuple<_T> &tup);
	private:
		T elem;
	};

	template<typename T>
	Tuple<T>::Tuple(Tuple &&tup) : elem()
	{
		using std::swap;
		swap(elem, tup.elem);
	}

	template<typename T>
	Tuple<T>& Tuple<T>::operator=(const Tuple &tup)
	{
		if (this != &tup)
			elem = tup.elem;
		return *this;
	}

	template<typename T>
	Tuple<T>& Tuple<T>::operator=(Tuple &&tup)
	{
		using std::swap;
		if (this != &tup)
			swap(elem, tup.elem);
		return *this;
	}

	template<size_t index, typename T, typename... Args>
	struct Element_type<index, Tuple<T, Args...>>
	{
		typedef typename std::conditional<sizeof...(Args) == 0, T, typename Element_type<index - 1, Tuple<Args...>>::type>::type type;
	};

	//enables range checking
	template<size_t index>
	struct Element_type<index, Tuple<>>
	{
		typedef void type;
	};

	template<typename T, typename... Args>
	struct Element_type<0, Tuple<T, Args...>>
	{
		typedef T type;
	};

	//for the std::pair container
	template<typename T, typename U>
	struct Element_type<0, std::pair<T, U>>
	{
		typedef T type;
	};

	template<typename T, typename U>
	struct Element_type<1, std::pair<T, U>>
	{
		typedef U type;
	};

	template<typename T, typename... Args>
	template<size_t index, typename... _Args>
	const typename Element_type<index, Tuple<_Args...>>::type& Tuple<T, Args...>::get(const Tuple<_Args...> &tup)
	{
		using ret_type = const typename Element_type<index, Tuple<_Args...>>::type;

		if (index == 0)
			return *(reinterpret_cast<ret_type*>(&tup.elem));
		else
			return *(reinterpret_cast<ret_type*>(&decltype(tup.next)::get<index - 1>(tup.next)));
	}

	template<typename T>
	template<size_t index, typename _T>
	const typename Element_type<index, Tuple<_T>>::type& Tuple<T>::get(const Tuple<_T> &tup)
	{
		if (index == 0)
			return tup.elem;
		else
			throw std::out_of_range("Tuple index out of range");
	}

	template<size_t index, typename T, typename U>
	const typename Element_type<index, std::pair<T, U>>::type& get(const std::pair<T, U> &pr)
	{
		using ret_type = const typename Element_type<index, std::pair<T, U>>::type;

		if (index == 0)
			return *(reinterpret_cast<ret_type*>(&pr.first));
		else if (index == 1)
			return *(reinterpret_cast<ret_type*>(&pr.second));
		else
			throw std::out_of_range("Pair index out of range");
	}

	template<typename... Args>
	Tuple<Args...> make_tuple(Args&&... args)
	{
		return Tuple<Args...>(std::forward<Args>(args)...);
	}

	template<typename Tuple_type, typename... Preds>
	struct Tuple_compare : Tuple<Preds...>
	{
		Tuple_compare() = default;
		Tuple_compare(const Preds&... args) : Tuple(args...) {}
		
		template<size_t index>
		bool compare(const Tuple_type &lhs, const Tuple_type &rhs) const
		{
			return Tuple::get<index>(*this)(Tuple_type::get<index>(lhs), Tuple_type::get<index>(rhs));
		}
		
		static constexpr size_t dimension() { return sizeof...(Preds); }
	};

	template<template<typename...> typename Comp = std::less, typename... Args>
	Tuple_compare<Comp<Args>...> make_uniform_compare(Tuple<Args...> tup)
	{
		return Tuple_compare<Comp<Args>...>();
	}

	template<template<typename...> typename... Predicates, typename... Args>
	Tuple_compare<Predicates<Args>...> make_nonuniform_compare(const Tuple<Args...> &tup)
	{
		//static_assert(sizeof...(Predicates) == sizeof...(Args), "Invalid number of arguments to make_uniform_compare");
		return Tuple_compare<Predicates<Args>...>(Predicates<Args>()...);
	}
}