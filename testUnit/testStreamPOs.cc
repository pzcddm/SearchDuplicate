#include <iostream>
#include <fstream>
#include <execution>
#include <bits/stdc++.h>
using namespace std;
int main(){
    vector<int> a(100000000);
    srand(0);
    for(int i =0 ;i<100000000;i++){
        a[i] = rand();
    }
    std::sort(std::execution::par_unseq, a.begin(), a.end());
    // sort(a.begin(), a.end());
}