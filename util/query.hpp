#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "new_utils.hpp"
#include "utils.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern unordered_map<string, int> word2id;
extern unordered_set<string> stopwords;
extern vector<unorder_map<int, vector<CW>>> k_hashMapAllWords; // todo put these cw vectors into files

class Query
{
    string sequence;
    float theta; // threshold that is lower than 1
    int k;       // the num of hash functions that use

public:
    Query() {}

    Query(string _seq, float _theta, int _k) : sequence(_sequence), theta(_theta), k(_k)
    {
        assert(theta <= 1.0);
    }

    // Todo
    vector<CW> getResult()
    {
        vector<int> minHashes;
        map<int, vector<CW>> &doc_groups;

        // get these K minHashes of the query sequence
        getKMinHash(minHashes);
        // Group those compat window vectors by T (document id)
        GroupT(doc_groups, minHashes);

        // Implement Find Subset algorithm to each group and filter those whose size is lower than ceil(theta*k)
        int thres = int(ceil(k * theta));
        for (auto const &gp : doc_groups)
        {
            // filter each group's size
            if (gp.second.size() < thres)
                continue;
            
            // Todo get the result of Subset algorithm
        }
    }

private:
    void getKMinHash(vector<int> &minHashes)
    {
        // Prepocess sequence
        vector<int> seqTokenized();
        vector<int> offset();
        seq2Int(sequence, seqTokenized, offset, word2id, stopwords); // tokenize the query sequence

        vector<int> hashValues(seqTokenized.size());
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < seqTokenized.size(); j++)
            {
                hashValues[j] = hval(hashFunctions, seqTokenized[j], i);
            }

            // Get minHash of current hashfunction
            int minValue = *min_element(hashValues.begin(), hashValues.end());
            minHashes.push_back(minValue);
        }

        return;
    }

    // Group those compat window vectors by T (document id)
    void GroupT(map<int, vector<CW>> &groups, const vector<int> &minHashes)
    {
        // load the coresponding cw vectors of these K minHashes
        // and group them by document id
        for (int i = 0; i < minHashes.size(); i++)
        {
            int key = minHashes[i];
            vector<CW> &cw_vet = k_hashMapAllWords[i][key];
            for (auto &cw : cw_vet)
            {
                int doc_id = cw.T;
                if (groups.count(doc_id))
                {
                    groups[doc_id].push_back(cw);
                }
                else
                {
                    vector<cw> &tmp_vet = groups[doc_id];
                    tmp_vet.push_back(cw);
                    assert(groups[doc_id].size() > 0); // todo to delete just to check
                }
            }
        }
    }
}