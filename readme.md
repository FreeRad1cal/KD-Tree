# A heterogeneously typed KD-Tree implemented in C++

What makes this KD-Tree library different? It allows the use of different object types for the dimensions of the search key. Further, it allows the user to supply an arbitrary comparer for each dimension of the key. The additional flexibility makes this library useful for nonconventional applications of the KNN search.

## Getting Started

The library was written in Visual C++ with Visual Studio, but you should have no problem using it on other platforms.

## Using the library

Include the KD_tree.h header file:
```c++
#include "KD_tree.h"
```
The KD_tree class template is instantiated as follows:
```c++
auto kd_tree = BK_KD_tree::KD_tree<3, std::string, BK_KD_tree::Comparer_wrapper<std::less, std::less, std::less>, BK_KD_tree::Type_wrapper<int, int, std::string>, false>();
```
The first template parameter (`Dim`) specifies the number of dimensions for the search key - in this case `3`. The second template parameter (`Mapped`) specifies the type of the mapped type - in this case `std::string`. The third template parameter (`PredWrapper`) specifies the predicate types that are to be used for each of the dimensions. These must be wrapped in the helper `BK_KD_tree::Comparer_wrapper` template. The fourth template parameter (`DimWrapper`) specifies the types for the dimensions. Likewise, these must be wrapped in the `BK_KD_tree::Type_wrapper` template. The number of arguments to `Comparer_wrapper` and `Type_wrapper` templates must match the number of dimensions. The only exception is when all dimensions either have the same type or use the same predicate, in which case the wrappers can be instantiated with only one argument. The last template parameter (`Mfl`) currently does not do anything.

A set of default copy and move constructors and assignment operators are also provided.

The library offers the following basic set of operations:
``` 
insert
erase
operator[]
at
size
clear
contains
KNN_search
```

#### insert
```c++
auto value = kd_tree.insert("foo", 1, 2, "str_key");
```
The first argument to `insert` is the mapped value, followed by the key coordinates. The operation returns the inserted key-value pair. If the key already exists, the current value is overwritten.

#### erase
```c++
decltype(kd_tree)::key_type key_type;
auto result = kd_tree.erase(key_type(1, 2, "str_key"));
```
The `erase` method returns the number of items deleted (`1` if succeeded, `0` if key does not exist).

#### operator[]
```c++
decltype(kd_tree)::key_type key_type;
auto value = kd_tree[key_type(1, 2, "str_key")];
```
The index operator returns the current mapped value or inserts a default value.

#### at
```c++
decltype(kd_tree)::key_type key_type;
auto value = kd_tree[key_type(1, 2, "str_key")];
```
The `at` method return the current mapped value or throws a `not_found` exception if the key does not exist.

#### size
```c++
kd_tree.size();
```
The `size` method returns the number of nodes in the tree.

#### clear
```c++
kd_tree.clear();
```
The `clear` method recursively deletes all nodes of the tree. It is invoked by the KD_tree destructor.

#### contains
```c++
auto contains = kd_tree.contains(key_type(1, 2, "str_key"));
```
The `contains` method returns a boolean value that indicates whether the key exists.

#### KNN_search
```c++
auto kd_tree = BK_KD_tree::KD_tree<3, std::string, BK_KD_tree::Comparer_wrapper<std::less, std::less, std::less>, BK_KD_tree::Type_wrapper<int, int, double>, false>();
typedef decltype(kd_tree)::key_type key_type;
std::default_random_engine random_engine;
for (auto i = 0; i < 100000; ++i)
{
    if (i == 50000)
        kd_tree[key_type(301, 501, 601)] = "needle";
    kd_tree.insert(std::string("hay") + std::to_string(i), random_engine() % 10001, random_engine() % 10001, random_engine() % 10001);
}

auto distanceCalculator = DistanceCalculator<key_type>();
auto result = tree.KNN_search(1, distanceCalculator, key_type(300, 500, 600));
```
The `KNN_search` method takes three arguments: the number of nearest neighbors to locate, an object that computes the distance between two coordinates (keys), and the input coordinate. The method returns an `std::vector` of nearest neighbors where each result is a `BK_Tuple::Tuple` with the cartesian distance to the input coordinate and the mapped value. The aforementioned `Tuple` class template is similar in functionality to `std::Tuple` but created specifically for this project. The following ia an example implementation of `DistanceCalculator` that will work for types that define `operator-`:
```c++
template<typename T>
struct DistanceCalculator
{
public:
    double get_cartesian_distance(const T &key1, const T &key2) const
    {
        return _get_cartesian_distance<T::dimension() - 1>(key1, key2);
    }

    template<size_t N>
    double get_distance_to_plane(const T &key1, const T &key2) const
    {
        return T::get<N>(key1) > T::get<N>(key2) ? T::get<N>(key1) - T::get<N>(key2) : T::get<N>(key2) - T::get<N>(key1);
    }

private:
    template<size_t N>
    double _get_cartesian_distance(const T &key1, const T &key2) const
    {
        auto coord1 = T::get<N>(key1), coord2 = T::get<N>(key2);
        double distance = coord1 > coord2 ? coord1 - coord2 : coord2 - coord1;
        return (distance * distance) + _get_cartesian_distance<N - 1>(key1, key2);
    }

    template<>
    double _get_cartesian_distance<0>(const T &key1, const T &key2) const
    {
        auto coord1 = T::get<0>(key1), coord2 = T::get<0>(key2);
        double distance = coord1 > coord2 ? coord1 - coord2 : coord2 - coord1;
        return (distance * distance);
    }
};
```
The `get_cartesian_distance` and `get_distance_to_plane` methods are required by KD_tree class.
