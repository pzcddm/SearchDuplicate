include <fstream>
#include <cmath>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <unordered_set>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <set>
#include "omp.h"
#include "segtree.h"
#include <dirent.h>
#include <chrono>
#include "utils.hpp"

using namespace std;
#define upper_l(id) (id<<2) -2
#define upper_r(id) (id << 2) - 1
#define bottom_l(id) (id << 2)
#define bottom_r(id) (id << 2) | 1

class CW
{
    public:
    int l; // left boundary
    int r; // right boundary
    int c; // center
    int T; // document id

    CW(){}

    CW(const int &_l, const int &_r, const int &_c, const int &T_):l(_l), r(_r), c(_c), T(_T){}

    void display() const {
      printf("(%d, %d, %d, %d)\n", l, r, c, T);
    }
}

class SegmentTree2D{
    struct Node{
        short x_l,x_r,y_l,y_r;
        int count; //count how many triangles overlaps in this node(sub triangle)
        int lazy; //lazy tag
    }s
    int docLen;
    int limit;
    
    
}

