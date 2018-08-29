#include "stdafx.h"
#include "CppUnitTest.h"
#include "../KD_tree/KD_tree_node.h"
#include "../KD_tree/KD_tree_point.h"
#include "../KD_tree/KD_tree.h"
#include "../KD_tree//tuple.h"
#include <string>
#include <iostream>
#include <functional>
#include <random>
#include <cmath>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace KD_treeTests
{	
	using namespace BK_KD_tree;

	template<typename T>
	struct DistanceCalculator
	{
	public:

		DistanceCalculator(size_t &op_count) : _op_count(op_count)
		{
		}

		double cartesian(const T &key1, const T &key2) const
		{
			++_op_count;
			return _cartesian<T::dimension() - 1>(key1, key2);
		}

		template<size_t N>
		double _cartesian(const T &key1, const T &key2) const
		{
			auto coord1 = T::get<N>(key1), coord2 = T::get<N>(key2);
			double distance = coord1 > coord2 ? coord1 - coord2 : coord2 - coord1;
			return (distance * distance) + _cartesian<N - 1>(key1, key2);
		}

		template<>
		double _cartesian<0>(const T &key1, const T &key2) const
		{
			auto coord1 = T::get<0>(key1), coord2 = T::get<0>(key2);
			double distance = coord1 > coord2 ? coord1 - coord2 : coord2 - coord1;
			return (distance * distance);
		}

		template<size_t N>
		double to_plane(const T &key1, const T &key2) const
		{
			return T::get<N>(key1) > T::get<N>(key2) ? T::get<N>(key1) - T::get<N>(key2) : T::get<N>(key2) - T::get<N>(key1);
		}

	private:
		size_t &_op_count;
	};

	TEST_CLASS(Tests)
	{
	private:

		std::default_random_engine random_engine;
		KD_tree<3, std::string, Comparer_wrapper<std::less>, Type_wrapper<int, int, double>, false> tree;

		typedef std::default_random_engine::result_type random_type;
		typedef decltype(tree)::key_type key_type;
		typedef decltype(tree)::key_compare key_compare;

	public:

		TEST_METHOD(KNN_search_ShouldFindNearestNeighbor)
		{
			for (auto i = 0; i < 100000; ++i)
			{
				if (i == 50000)
					tree[key_type(301, 501, 601)] = "needle";
				tree.insert(std::string("hay") + std::to_string(i), random_engine() % 10001, random_engine() % 10001, random_engine() % 10001);
			}

			size_t op_count = 0;
			auto distanceCalculator = DistanceCalculator<key_type>(op_count);
			auto res = tree.KNN_search(1, distanceCalculator, key_type(300, 500, 600));
			Assert::IsTrue(res.size() == 1);
			Assert::IsTrue(*(res[0].second) == "needle");
		}

		TEST_METHOD(KNN_search_ShouldHaveTheCorrectComplexity)
		{
			for (auto i = 0; i < 100000; ++i)
			{
				if (i == 50000)
					tree[key_type(301, 501, 601)] = "needle";
				tree.insert(std::string("hay") + std::to_string(i), random_engine() % 10001, random_engine() % 10001, random_engine() % 10001);
			}

			size_t op_count = 0;
			auto distanceCalculator = DistanceCalculator<key_type>(op_count);
			auto res = tree.KNN_search(1, distanceCalculator, key_type(300, 500, 600));
			Assert::IsTrue(res.size() == 1);
			Assert::IsTrue(*(res[0].second) == "needle");
			Assert::IsTrue(op_count < 100);
		}
	};
}