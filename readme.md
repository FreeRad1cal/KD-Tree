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
The first template parameter (`Dim`) specifies the number of dimensions for the search key - in this case 3. The second template parameter (`Mapped`) specifies the type of the mapped type - in this case `std::string`. The third template parameter (`PredWrapper`) specifies the predicate types that are to be used for each of the dimensions. These must be wrapped in the helper `BK_KD_tree::Comparer_wrapper` template. The fourth template parameter (`DimWrapper`) specifies the types for the dimensions. Likewise, these must be wrapped in the `BK_KD_tree::Type_wrapper` template. The number of arguments to `Comparer_wrapper` and `Type_wrapper` templates must match the number of dimensions. The only exception is when either all dimensions have the same type or use the same predicate, in which case the wrappers can be instantiated with only one argument. Lastly, the last template parameter (`Mfl`) specifies whether the tree should allow duplicate keys (currently not implemented).

A set of default copy and move constructors and assignment operators are also provided.

The library offers the following basic set of operations:
``` 
insert
erase
find
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
decltype(tree)::key_type key_type;
auto value = kd_tree.erase(key_type(1, 2, "str_key"));
```


## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Billie Thompson** - *Initial work* - [PurpleBooth](https://github.com/PurpleBooth)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc

