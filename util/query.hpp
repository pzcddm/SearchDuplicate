#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "indexItem.hpp"
#include "new_utils.hpp"
#include "utils.hpp"
#include "nearDupSearch.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern unordered_map<string, int> word2id;
extern unordered_set<string> stopwords;
extern IndexItem **indexArr;
extern int wordNum;

class Query {
    string sequence;
    float theta; // threshold that is lower than 1
    int k;       // the num of hash functions that use

public:
    Query() {
    }

    Query(string _seq, float _theta, int _k) :
        sequence(_seq), theta(_theta), k(_k) {
        assert(theta <= 1.0);
    }

    // Todo
    vector<CW> getResult() {
        vector<CW> res;
        vector<int> minHashesToken;             // save the tokenid that has minHashValue with each hashfunction
        map<int, vector<CW>> doc_groups;        // group CWs by their document id

        // get these K minHashes of the query sequence
        getKMinHash(minHashesToken);
        
        // Group those compat window vectors by T (document id)
        GroupT(doc_groups, minHashesToken);
    
        // Implement Find Subset algorithm to each group and filter those whose size is lower than ceil(theta*k)
        int thres = int(ceil(k * theta));
        for (auto const &gp : doc_groups) {
            // filter each group's size
            if (gp.second.size() < thres)
                continue;

            nearDupSearch(gp.second, thres, res); 
            // Todo get the result of Subset algorithm
        }

        return res;
    }

private:
    void getKMinHash(vector<int> &minHashesToken) {
        // Prepocess sequence
        vector<int> seqTokenized;
        vector<int> offset;
        seq2Int(sequence, seqTokenized, offset, word2id, stopwords); // tokenize the query sequence

        for (int i = 0; i < k; i++) {
            vector<int> hashValues(seqTokenized.size());
            for (int j = 0; j < seqTokenized.size(); j++) {
                hashValues[j] = hval(hashFunctions, seqTokenized[j], i);
            }

            // Get minHash of current hashfunction
            int minValuePos = min_element(hashValues.begin(), hashValues.end()) - hashValues.begin();
            minHashesToken.push_back(seqTokenized[minValuePos]);
        }
    }

    // Group those compat window vectors by T (document id)
    void GroupT(map<int, vector<CW>> &groups, const vector<int> &minHashesToken) {
        // load the coresponding cw vectors of these K minHashes
        // and group them by document id

        assert(minHashesToken.size() ==k);
        for (int i = 0; i < minHashesToken.size(); i++) {
            
            int token_id = minHashesToken[i];
            assert(token_id>=0 && token_id <wordNum);
            IndexItem & indexItem = indexArr[i][token_id];
            vector<CW> cw_vet;
            indexItem.getCompatWindows("CompatWindows",cw_vet,token_id);

            
            // group them by their document id
            for (auto &cw : cw_vet) {
                int doc_id = cw.T;
                if (groups.count(doc_id)) {
                    groups[doc_id].push_back(cw);
                } else {
                    vector<CW> &tmp_vet = groups[doc_id];
                    tmp_vet.push_back(cw);
                    assert(groups[doc_id].size() > 0); //will delete (just to check)
                }
            }
        }
    }
};