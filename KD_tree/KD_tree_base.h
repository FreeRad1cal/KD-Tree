#pragma once
#include <utility>
#include <stdexcept>
#include <vector>
#include <type_traits>
#include "KD_tree_node.h"
#include "KD_tree_iterator.h"

namespace BK_KD_tree
{
	class not_found : public std::runtime_error
	{
	public:
		using runtime_error::runtime_error;
	};

	template<typename Traits>
	class KD_tree_base;

	namespace detail
	{
		template<typename T, typename U = std::decay_t<T>, typename... Args>
		struct is_KD_tree_base : std::false_type {};

		template<typename T, typename Traits>
		struct is_KD_tree_base<T, KD_tree_base<Traits>> : std::true_type {};
	}

	template<typename Traits>
	class KD_tree_base
	{
	public:
		typedef typename Traits::key_type		key_type;
		typedef typename Traits::mapped_type	mapped_type;
		typedef typename Traits::value_type		value_type;
		typedef typename Traits::size_type		size_type;
		typedef typename Traits::key_compare	key_compare;
		static constexpr bool Multi = Traits::Multi;
		static constexpr size_t Dim = Traits::Dimension;

		KD_tree_base() : m_root(nullptr), m_comp() {}
		explicit KD_tree_base(const key_compare &compare) : m_root(nullptr), m_comp(compare) {}
		KD_tree_base(const KD_tree_base &tree) : m_root(copy_tree_op(tree.m_root)), m_comp(tree.m_comp) {}
		KD_tree_base(KD_tree_base &&tree);

		KD_tree_base& operator=(const KD_tree_base &tree);
		KD_tree_base& operator=(KD_tree_base &&tree);

		~KD_tree_base() { clear(); }

		value_type& insert(const value_type &value);
		value_type& insert(value_type &&value);
		size_t erase(const key_type &key);

		bool empty() const { return m_root == nullptr; }
		size_t size() const { return size_op(m_root); }
		static constexpr size_t dimension() { return Dim; }
		void clear() { destroy_tree_op(m_root); }

	protected:
		typedef KD_tree_node<Traits> node_type;
		typedef node_type* node_pointer;
		typedef const node_type* const_node_pointer;

		node_pointer	m_root;
		key_compare		m_comp;

		//Advances the dimension index
		template<size_t N>
		static constexpr size_type next_dim() { return (N + 1) % Dim; }

		//Overload set for testing two key types for equality
		bool compare_keys(const key_type &lhs, const key_type &rhs) const { return _compare_keys<0>(lhs, rhs); }
		template<size_t N>
		bool _compare_keys(const key_type &lhs, const key_type &rhs) const;
		template<>
		bool _compare_keys<Dim>(const key_type &lhs, const key_type &rhs) const { return true; }

		value_type& find(const key_type &key) { return find_op<0>(m_root, key)->value(); }
		const value_type& find(const key_type &key) const { return find_op<0>(m_root, key)->value(); }

		//Swaps two nodes
		void swap_nodes(node_pointer &a, node_pointer &b);
		//Recursively finds the size of the tree
		size_t size_op(const node_pointer root) const;
		//Returns a reference to the node with the given key or throws an exception
		template<size_t N>
		node_pointer& find_op(node_pointer &current, const key_type &key);
		template<size_t N>
		const_node_pointer find_op(const_node_pointer current, const key_type &key) const;
		//Locates the given point and calls erase with the proper dimension index
		template<size_t N>
		size_t find_erase(node_pointer &curent, const key_type &key);
		//Finds the insert location for a new node
		template<size_t N>
		node_pointer& insert_loc_op(node_pointer &current, const key_type &new_key);
		//Erases a node by reconstructing the subtree
		template<size_t N>
		size_t erase_op(node_pointer &current);
		//Converts a subtree to an array of nodes in preorder
		void to_arr_preorder(node_pointer &current, std::vector<node_pointer> &arr);
		//Recursively copies a tree
		node_pointer copy_tree_op(const const_node_pointer source_root);
		//Recursively deallocates a tree
		size_t destroy_tree_op(node_pointer &current);
	};

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	KD_tree_base<Traits>::KD_tree_base(KD_tree_base &&tree) : m_root(nullptr), m_comp()
	{
		using std::swap;
		std::swap(m_root, tree.m_root);
		swap(m_comp, tree.m_comp);
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	KD_tree_base<Traits>& 
	KD_tree_base<Traits>::operator=(const KD_tree_base &tree)
	{
		if (&tree != this)
		{
			clear();
			m_root = copy_tree_op(tree.m_root);
			m_comp = tree.m_comp;
		}

		return *this;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	KD_tree_base<Traits>&
	KD_tree_base<Traits>::operator=(KD_tree_base &&tree)
	{
		if (&tree != this)
		{
			using std::swap;
			std::swap(m_root, tree.m_root);
			m_comp = std::move(m_comp);
		}

		return *this;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	size_t 
	KD_tree_base<Traits>::destroy_tree_op(node_pointer &current)
	{
		if (current != nullptr)
		{
			//postorder traversal
			size_t res_left = destroy_tree_op(current->left_child());
			size_t res_right = destroy_tree_op(current->right_child());
			delete current;
			current = nullptr;
			return 1 + res_left + res_right;
		}
		else
			return 0;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	bool
	KD_tree_base<Traits>::_compare_keys(const key_type &lhs, const key_type &rhs) const
	{
		static_assert(N < Dim, "Invalid arguments to compare_keys template parameter");

		//if all dimensions of lhs compare equal to all dimensions of rhs
		if (!m_comp.compare<N>(lhs, rhs) && !m_comp.compare<N>(rhs, lhs))
		{
			//if N is not the last dimension
			if (N + 1 < Dim)
				return _compare_keys<N + 1>(lhs, rhs);
			else //the end of the key has been reached and all dimensions compare equal
				return true;
		}
		else //a pair of nonequal dimensions has been found
			return false;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	typename KD_tree_base<Traits>::node_pointer&
		KD_tree_base<Traits>::insert_loc_op(node_pointer &current, const key_type &new_key)
	{
		if (current == nullptr || compare_keys(Traits::val_to_key(current->value()), new_key)) //if the current node is null or its key compares equal to new_key
			return current;
		else if (m_comp.compare<N>(new_key, Traits::val_to_key(current->value())))
			return insert_loc_op<next_dim<N>()>(current->left_child(), new_key);
		else
			return insert_loc_op<next_dim<N>()>(current->right_child(), new_key);
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	typename KD_tree_base<Traits>::value_type&
	KD_tree_base<Traits>::insert(const value_type &value)
	{
		node_pointer &insert_loc = insert_loc_op<0>(m_root, Traits::val_to_key(value));

		if (insert_loc == nullptr) //If no equivalent key exists in the tree, insert a new leaf
			insert_loc = new node_type(value_type(value));
		else //If a key with the given coordinates already exists, replace the mapped value
			insert_loc->value() = value;

		return insert_loc->value();
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	typename KD_tree_base<Traits>::value_type&
	KD_tree_base<Traits>::insert(value_type &&value)
	{
		node_pointer &insert_loc = insert_loc_op<0>(m_root, Traits::val_to_key(value));

		if (insert_loc == nullptr) //If no equivalent key exists in the tree, insert a new leaf
			insert_loc = new node_type(value_type(std::move(value)));
		else //If a key with the given coordinates already exists, replace the mapped value
			insert_loc->value() = std::move(value);

		return insert_loc->value();
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	void 
	KD_tree_base<Traits>::swap_nodes(node_pointer &a, node_pointer &b)
	{
		node_pointer a_left = a->left_child(), a_right = a->right_child();
		a->left_child() = b->left_child();
		a->right_child() = b->right_child();
		b->left_child() = a_left;
		b->right_child() = a_right;
		std::swap(a, b);
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	size_t 
	KD_tree_base<Traits>::erase_op(node_pointer &current)
	{
		//a temporary vector to store the nodes
		std::vector<node_pointer> temp;
		//a variable to store the root of the reconstructed subtree
		node_pointer subtree_root = nullptr;
		
		//convert to subtree to a vector of nodes
		if (current->left_child() != nullptr)
			to_arr_preorder(current->left_child(), temp);
		else if (current->right_child() != nullptr)
			to_arr_preorder(current->right_child(), temp);

		//reconstruct the subtree
		for (auto it = temp.begin(), end_it = temp.end(); it != end_it; ++it)
		{
			node_pointer &insert_loc = insert_loc_op<N>(subtree_root, Traits::val_to_key((*it)->value()));
			insert_loc = *it;
		}

		//delete the old root of the subtree
		delete current;
		current = subtree_root;
		return 1;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	typename KD_tree_base<Traits>::node_pointer&
	KD_tree_base<Traits>::find_op(node_pointer &current, const key_type &key)
	{
		if (current != nullptr && !compare_keys(Traits::val_to_key(current->value()), key))
		{
			if (m_comp.compare<N>(key, Traits::val_to_key(current->value())))
				return find_op<next_dim<N>()>(current->left_child(), key);
			else
				return find_op<next_dim<N>()>(current->right_child(), key);
		}
		else if (current == nullptr)
			throw not_found("Key not found");
		else
			return current;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	typename KD_tree_base<Traits>::const_node_pointer 
	KD_tree_base<Traits>::find_op(const_node_pointer current, const key_type &key) const
	{
		if (current != nullptr && !compare_keys(Traits::val_to_key(current->value()), key))
		{
			if (m_comp.compare<N>(key, Traits::val_to_key(current->value())))
				return find_op<next_dim<N>()>(current->left_child(), key);
			else
				return find_op<next_dim<N>()>(current->right_child(), key);
		}
		else if (current == nullptr)
			throw not_found("Key not found");
		else
			return current;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	template<size_t N>
	size_t
	KD_tree_base<Traits>::find_erase(node_pointer &current, const key_type &key)
	{
		if (current != nullptr && !compare_keys(Traits::val_to_key(current->value()), key))
		{
			if (m_comp.compare<N>(key, Traits::val_to_key(current->value())))
				return find_erase<next_dim<N>()>(current->left_child(), key);
			else if (m_comp.compare<N>(Traits::val_to_key(current->value()), key))
				return find_erase<next_dim<N>()>(current->right_child(), key);
		}
		else if (current == nullptr)
			return 0;
		else
			return erase_op<N>(current);
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	void 
	KD_tree_base<Traits>::to_arr_preorder(node_pointer &current, std::vector<node_pointer> &arr)
	{
		if (current != nullptr)
		{
			arr.push_back(current);
			to_arr_preorder(current->left_child(), arr);
			to_arr_preorder(current->right_child(), arr);
			current = nullptr;
		}
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	size_t
	KD_tree_base<Traits>::erase(const key_type &key)
	{
		return find_erase<0>(m_root, key);
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	size_t
	KD_tree_base<Traits>::size_op(const node_pointer root) const
	{
		if (root != nullptr)
		{
			size_t left_res = size_op(root->left_child());
			size_t right_res = size_op(root->right_child());
			return 1 + left_res + right_res;
		}
		else
			return 0;
	}

	//---------------------------------------------------------------------------------------------

	template<typename Traits>
	typename KD_tree_base<Traits>::node_pointer
	KD_tree_base<Traits>::copy_tree_op(const const_node_pointer source_root)
	{
		if (source_root == nullptr)
			return nullptr;
		else
		{
			node_pointer new_node = new node_type(*source_root);
			new_node->left_child() = copy_tree_op(source_root->left_child());
			new_node->right_child() = copy_tree_op(source_root->right_child());
			return new_node;
		}
	}

	//---------------------------------------------------------------------------------------------
}