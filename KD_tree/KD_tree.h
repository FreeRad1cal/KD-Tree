/*
	- Use typename for dependent names only - unnecessary for instantiated templates
	- Any number of parameter packs can be specified in a class specialization, as long as these types are deduced from a specialized type
	- ADL does not find a function template when the arguments are explicitly specified
	- A templated constructor will be called instead of a copy assignment operator, because it is a better match due to lack of a const conversion
*/

#pragma once

#include "KD_tree_point.h"
#include "KD_tree_node.h"
#include "KD_tree_base.h"
#include "Priority_queue.h"
#include "tuple.h"
#include <type_traits>
#include <functional>
#include <typeinfo>

namespace BK_KD_tree
{
	template<typename... Args>
	struct Type_wrapper
	{
		static constexpr size_t dimension = sizeof...(Args);
	};

	//--------------------------------------------------------------------------------------------

	template<template<typename...> typename... PredT>
	struct Comparer_wrapper
	{
		static constexpr size_t dimension = sizeof...(PredT);
	};

	//--------------------------------------------------------------------------------------------

	namespace detail
	{
	//---------------------------------------------------------------------------------------------
		//Checks if all types in a non-empty parameter pack are the same. Syntax: are_same<Pack...>::value
		template<typename T, typename... Args>
		struct are_same;

		template<typename T, typename U, typename... Args>
		struct are_same<T, U, Args...> : std::conditional<are_same<T, U>::value, typename are_same<U, Args...>::type, std::false_type>::type
		{};

		template<typename T>
		struct are_same<T, T> : std::true_type {};

		template<typename T, typename U>
		struct are_same<T, U> : std::false_type {};

		template<typename T>
		struct are_same<T> : std::true_type {};

		template<typename T, typename... Args>
		struct grab_first_type { typedef T type; };

		template<template<typename...> typename T, template<typename...> typename... Args>
		struct grab_first_template { template<typename... Args> using type = T<Args...>; };

	//---------------------------------------------------------------------------------------------
		//Compares two parameter packs for equality. Syntax: compare_pack<Pack1...>::to<Pack2>::value or compare_pack<Pack1...>::to<Pack2>()
		template<typename T, typename... Args>
		struct compare_pack
		{
			//template<typename U, typename... Args2>
			//struct to : std::conditional<are_same<T, U>::value, typename compare_pack<Args...>::template to<Args2...>::type, std::false_type>::type {};

			template<typename U, typename... Args2>
			static constexpr bool to()
			{
				return are_same<T, U>::value ? 
					compare_pack<Args...>::template to<Args2...>() : false;
			}
		};

		template<typename T>
		struct compare_pack<T>
		{
			//template<typename U>
			//struct to : are_same<T, U>::type {};

			template<typename U>
			static constexpr bool to()
			{
				return are_same<T, U>::value;
			}
		};

		//Extracts type of key from template arguments
		template<typename T, typename U, typename... Args>
		struct get_key_type;

		//If the class is instantiated with a template wrapper with multiple parameters
		template<typename KeyT, template<typename...> typename... PredT, typename... Args>
		struct get_key_type<KeyT, Comparer_wrapper<PredT...>, Args...>
		{
			//Expansion pattern: Pred1<Arg1>, Pred2<Arg2>, ... , Predk<Argk>
			typedef BK_Tuple::Tuple_compare<KeyT, PredT<Args>...> type;
		};

		//If the class is instantiated with a template wrapper with a single parameter
		template<typename KeyT, template<typename...> typename PredT, typename... Args>
		struct get_key_type<KeyT, Comparer_wrapper<PredT>, Args...>
		{
			//Expansion pattern: Pred<Arg1>, Pred<Arg2>, ... , Pred<Argk>
			typedef BK_Tuple::Tuple_compare<KeyT, PredT<Args>...> type;
		};

		//If the class is instantiated with a type wrapper
		template<typename KeyT, typename... Preds, typename... Args>
		struct get_key_type<KeyT, Type_wrapper<Preds...>, Args...>
		{
			//Expansion pattern: Pred1, Pred2, ... , Predk
			typedef BK_Tuple::Tuple_compare<KeyT, Preds...> type;
		};

		//Returns true is distance_lhs - distance_rhs < 0
		template<typename T>
		struct queue_val_comp
		{
			bool operator()(const T &lhs, const T &rhs) const
			{
				return lhs.first < rhs.first;
			}
		};

		//A custom bounded priority queue class. T must be std::pair<double, value_type*>
		template<typename T, typename Container>
		class bounded_priority_queue : private BK_heap::Priority_queue<T, Container, queue_val_comp<T>>
		{
		public:
			typedef T value_type;
			
			//limit = 0 for unlimited size
			explicit bounded_priority_queue(size_t size_limit = 0) : Priority_queue(), lim(size_limit) {}
			using Priority_queue::top;
			using Priority_queue::pop;
			
			bool full() { return Priority_queue::size() == lim; }

			void push(const value_type &val)
			{
				if (lim > 0 && this->size() == lim)
					Priority_queue::replace(val);
				else
					Priority_queue::push(val);
			}

			void push(value_type &&val)
			{
				if (lim > 0 && this->size() == lim)
				{
					if (this->c(val, top()))
						Priority_queue::replace(std::move(val));
				}
				else
					Priority_queue::push(std::move(val));
			}

			Container& data() { return this->arr; }
			const Container& data() const { return this->arr; }
		private:
			size_t lim;
		};

	} //namespace detail

//---------------------------------------------------------------------------------------------
	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	struct KD_tree_traits;

	template<size_t Dim, typename Mapped, typename PredTypes, typename... DimTypes, bool Mfl>
	struct KD_tree_traits<Dim, Mapped, PredTypes, Type_wrapper<DimTypes...>, Mfl>
	{
		//Check the validity of the template arguments. Dim and the size of the DimTypes parameter pack must be > 0. 
		//If the sizes of DimTypes and Dim are not equal, then DimTypes must consist of only 1 parameter.
		static_assert(Dim > 0 && sizeof...(DimTypes) > 0 && (sizeof...(DimTypes) == Dim || sizeof...(DimTypes) == 1), "Invalid template arguments");
		//The size of the PredTypes parameter pack must either be 1 or must match the tree dimension
		static_assert(PredTypes::dimension > 0 && (PredTypes::dimension == 1 || PredTypes::dimension == Dim), "Invalid template arguments");

		typedef size_t								size_type;
		typedef Mapped								mapped_type;
		//Resolve the key type to either Point or Tuple
		typedef typename std::conditional<
								sizeof...(DimTypes) == 1 || detail::are_same<DimTypes...>::value,
								Point<Dim, typename detail::grab_first_type<DimTypes...>::type>,
								BK_Tuple::Tuple<DimTypes...>
									>::type			key_type;
		typedef std::pair<key_type, mapped_type>	value_type;

		//Resolve the multidimensional key comparison predicate
		typedef typename detail::get_key_type<key_type, PredTypes, DimTypes...>::type key_compare;

		//Extract key_type from value_type
		static const key_type& val_to_key(const value_type &val) { return val.first; }
		//Extract mapped_type from const value_type
		static const mapped_type& val_to_mapped(const value_type &val) { return val.second; }
		//Extract mapped_type from value_type
		static mapped_type& val_to_mapped(value_type &val) { return val.second; }

		//Multi-key allowed/disallowed flag
		static constexpr bool Multi = Mfl;

		static constexpr size_type Dimension = Dim;
	};

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	class KD_tree : public KD_tree_base<KD_tree_traits<Dim, Mapped, PredWrapper, DimWrapper, Mfl>>
	{
	private:
		typedef KD_tree_traits<Dim, Mapped, PredWrapper, DimWrapper, Mfl> tree_traits;
	public:
		typedef typename tree_traits::mapped_type				mapped_type;
		typedef typename tree_traits::key_type					key_type;
		typedef typename tree_traits::value_type				value_type;
		typedef typename tree_traits::size_type					size_type;
		typedef typename tree_traits::key_compare				key_compare;
		typedef typename std::pair<double, const mapped_type*>	KNN_type;
		typedef typename std::vector<KNN_type>					KNN_container_type;
		static constexpr bool Multi = tree_traits::Multi;

		KD_tree() = default;
		KD_tree(const KD_tree &tree) = default;
		KD_tree(KD_tree &&tree) = default;
		KD_tree(const key_compare &compare) : KD_tree_base(compare) {}
		template<typename InputIterator, typename... Preds>
		KD_tree(InputIterator begin, InputIterator end, Preds&&... predicates);

		~KD_tree() = default;
		KD_tree& operator=(const KD_tree &tree) = default;
		KD_tree& operator=(KD_tree &&tree) = default;

		template<typename... Coords>
		value_type& insert(const mapped_type &mapped, Coords&&... coordinates);
		template<typename... Coords>
		value_type& insert(mapped_type &&mapped, Coords&&... coordinates);
		template<typename... Coords>
		size_t erase(Coords&&... coordinates);

		mapped_type& operator[](const key_type &key);
		const mapped_type& operator[](const key_type &key) const;
		mapped_type& at(const key_type &key);
		const mapped_type& at(const key_type &key) const;
		bool contains(const key_type &key) const;

		template<typename Distance_op>
		KNN_container_type KNN_search(size_t k, Distance_op distance, const key_type &key) const;

	private:
		typedef KD_tree_node<tree_traits> node_type;
		typedef node_type* node_pointer;
		typedef const node_type* const_node_pointer;
		typedef detail::bounded_priority_queue<KNN_type, KNN_container_type> queue_type;

		template<size_t index, typename Distance_op>
		void KNN_search_op(const_node_pointer current, Distance_op &distance, const key_type &key, queue_type &q) const;
	};

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<typename InputIterator, typename ...Preds>
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::KD_tree(InputIterator begin, InputIterator end, Preds&&... predicates) : KD_tree_base(predicates...)
	{
		while (begin != end)
		{
			KD_tree_base::insert(*begin);
			++begin;
		}
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<typename... Coords>
	typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::value_type& 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::insert(const mapped_type &mapped, Coords&&... coordinates)
	{
		return KD_tree_base::insert(value_type{ key_type(std::forward<Coords>(coordinates)...), mapped });
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<typename... Coords>
	typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::value_type&
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::insert(mapped_type &&mapped, Coords&&... coordinates)
	{
		return KD_tree_base::insert(value_type{ key_type(std::forward<Coords>(coordinates)...), std::move(mapped) });
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::mapped_type& 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::operator[](const key_type &key)
	{
		try //check if a value with the given key exists
		{
			return tree_traits::val_to_mapped(KD_tree_base::find(key));
		}
		catch (...) //if not, insert a new value
		{
			return tree_traits::val_to_mapped(KD_tree_base::insert(value_type{key, mapped_type()}));
		}
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	const typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::mapped_type&
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::operator[](const key_type &key) const
	{
		//throw an exception if no value with given key exists
		return at(key);
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::mapped_type& 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::at(const key_type &key)
	{
		return tree_traits::val_to_mapped(KD_tree_base::find(key));
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	const typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::mapped_type&
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::at(const key_type &key) const
	{
		return tree_traits::val_to_mapped(KD_tree_base::find(key));
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	bool 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::contains(const key_type &key) const
	{
		try
		{
			KD_tree_base::find(key);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<typename... Coords>
	inline size_t KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::erase(Coords&&... coordinates)
	{
		return KD_tree_base::erase(key_type{std::forward<Coords>(coordinates)...});
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<typename Distance_op>
	typename KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::KNN_container_type 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::KNN_search(size_t k, Distance_op distance, const key_type &key) const
	{
		queue_type q(k);
		KNN_search_op<0>(this->m_root, distance, key, q);
		return std::move(q.data());
	}

//---------------------------------------------------------------------------------------------

	template<size_t Dim, typename Mapped, typename PredWrapper, typename DimWrapper, bool Mfl>
	template<size_t index, typename Distance_op>
	void 
	KD_tree<Dim, Mapped, PredWrapper, DimWrapper, Mfl>::KNN_search_op(const_node_pointer current, Distance_op &distance, const key_type &key, queue_type &q) const
	{
		//if a null node has been reached
		if (current == nullptr)
			return;

		//compute the distance from the current point to the test point (key)
		auto radius = distance.cartesian(tree_traits::val_to_key(current->value()), key);
		//push the result to the bounded priority queue
		q.push(KNN_type{ radius, &tree_traits::val_to_mapped(current->value()) });

		//recursively traverse the tree in the direction of the test point
		if (this->m_comp.compare<index>(key, tree_traits::val_to_key(current->value())))
			KNN_search_op<next_dim<index>()>(current->left_child(), distance, key, q);
		else
			KNN_search_op<next_dim<index>()>(current->right_child(), distance, key, q);

		//once a leaf has been reached, compute the distance to the test point
		auto dist_to_plane = distance.to_plane<index>(tree_traits::val_to_key(current->value()), key);
		//if the distance is smaller than the current largest distance in the queue, or if the queue is not full
		if (dist_to_plane < q.top().first || !q.full())
		{
			//check the other side of the splitting hyperplane for points that are closer
			//first need to find which side you are currently on
			if (this->m_comp.compare<index>(key, tree_traits::val_to_key(current->value())))
				KNN_search_op<next_dim<index>()>(current->right_child(), distance, key, q);
			else
				KNN_search_op<next_dim<index>()>(current->left_child(), distance, key, q);
		}
	}

	//template class KD_tree<3, std::string, Type_wrapper<std::greater<int>, std::greater<char>, std::less<double>>, Type_wrapper<int, char, double>, false>;
}