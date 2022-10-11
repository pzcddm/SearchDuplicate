# SearchDuplicate
The Search Part
## Description
1. To build all compat windows under k hash functions and store it into k distinct files into compatWindows directory.
2. Build index files which helps users get the position of the inverted compatWindows list under a specified ith hash function and a specified token id.
3. Given a query, search near duplicate sequences stored in compatWindows files with the help of index files.
## Compilation
```
// build compat windows files (if parrelled sort is needed, g++9 and relevant library are neccessary)
g++ -ggdb3 -O3 -w -Wextra -std=c++17 -pedantic -o buildCompatWindows buildCompatWindows.cc -ltbb -fopenmp![image](https://user-images.githubusercontent.com/50043231/194981958-5486e08a-f779-4a03-853d-fc3576863f6b.png)

// build index file
g++ -O3 buildIndex buildIndex.cc

// search Duplicate
g++ -ggdb3 -O3 -w -Wextra -std=c++17 -pedantic -o searchDuplicate searchDuplicate.cc -ltbb -fopenmp
'''
