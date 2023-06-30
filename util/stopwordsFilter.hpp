#pragma once
#include <bits/stdc++.h>
#include "IO.hpp"
#include "utils.hpp"
using namespace std;

/*
    This is a class that help filter the stopwords only given the path of bin that stores stopwords tokens
*/
class StopwordsFilter{
public:
    unordered_set<int> stopwords;

    StopwordsFilter(){}

    void load_stopwords(string stopwords_bin_path){
        printf("Loadind stopwords from %s\n", stopwords_bin_path.c_str());
        vector<int> vec;
        // load the stopwords
        loadBin(stopwords_bin_path, vec);
        for(auto & word: vec){
            stopwords.insert(word);
        }
    }

    StopwordsFilter(string stopwords_bin_path){
        vector<int> vec;
        // load the stopwords
        loadBin(stopwords_bin_path, vec);
        for(auto & word: vec){
            stopwords.insert(word);
        }
    }

    void const filter_erase(vector<int> & seq){
        auto it = std::remove_if(seq.begin(), seq.end(), 
        [this](const int& element){
            return this->stopwords.find(element) != this->stopwords.end();
        });
        seq.erase(it, seq.end());
    }

    void const filtered_hash(vector<int> & seq,  pair<int,int> & hf){
        std::for_each(seq.begin(), seq.end(), 
        [this, &hf](int& element){
            if(this->stopwords.find(element) != this->stopwords.end()){
                element = INT_MAX;
            }else{
                element = hval(hf, element);
            }
        });
    }

    int const filtered_hash(int element, const pair<int,int> & hf){
        if(this->stopwords.find(element) != this->stopwords.end()){
            return INT_MAX;
        }else{
            return hval(hf, element);
        }
    }

    // check if all the indexes of stopwords don't have any compactwindows
    bool check_stopwords_index(BigIndexItem **indexArr, const Config & config){
        for(int i = 0; i<config.k;i++){
            for(auto const & stop_word : stopwords){
                if(indexArr[i][stop_word].windowsNum > 0)
                    return false;
            }
        }
        return true;
    }
};