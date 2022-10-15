#include <iostream>
#include <fstream>
#include "../util/utils.hpp"
#include <execution>
#include <bits/stdc++.h>
using namespace std;

map<int,int> mp;
int total = 0;
int INTERVAL_LIMIT =50;
void partition( vector<int> &hashValues, int l, int r ) {
    if (r- l +1  <= INTERVAL_LIMIT)
        return;

    int minHash = hashValues[l];
    int min_pos = l;
    for (int i = l + 1; i <= r; i++) {
        if (minHash > hashValues[i]) {
            minHash = hashValues[i];
            min_pos = i;
        }
    }
    mp[r-l+1]++;
    total++;
    partition( hashValues, l, min_pos - 1);
    partition( hashValues, min_pos + 1, r);
}

int main(){
    vector<int> a(1000000);
    srand(0);
    for(int i =0 ;i<1000000;i++){
        a[i] = rand();
    }

    vector<pair<int, int>> hf;
    generateHashFunc(0, hf);
    for(int i =0 ;i<1000000;i++){
        a[i] = hval(hf, a[i], 0);
    }

    partition(a, 0, int(1000000 - 1));
    
    for(const auto & it : mp){
        printf("%d : %d \n",it.first,it.second);
    }
    printf("total:%d\n",total);
}