/*
A discreization for an array and create an unordered_map to map the value to its index
If there are duplicate maximums, the position is that of the rightest one.
*/

#pragma once
#include <bits/stdc++.h>
using namespace std;

class Discret{
    vector<int> arr;
    unordered_map<int,int> mp;

    public:
    void push_arr(const int & tmp){
        arr.emplace_back(tmp);
    }

    void exe(){
        sort(arr.begin(),arr.end());
        auto new_end = unique(arr.begin(),arr.end());
        arr.erase(new_end,arr.end());
        int cnt = 1;
        for(auto const & tmp: arr){
            mp[tmp] = cnt++;
        }
        
    }

    int getArrSize(){
        if(arr.size()>50000)
            cout<<"arr size:"<<arr.size()<<endl;
        return arr.size();
    }

    int discrete(const int& key){
        if(mp.count(key)==0){
            printf("Error , the key is not in this map\n");
        }
        return mp[key];
    }

    int rev_discret(const int & value){
        assert(value<arr.size());
        return arr[value];
    }
};