#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <omp.h>

#include "indexItem.hpp"
#include "new_utils.hpp"
#include "utils.hpp"
#include "nearDupSearch.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern IndexItem **indexArr;
extern int wordNum;

class Query {
    vector<int> seqTokenized;
    float theta;    // threshold that is lower than 1
    int k;          // the num of hash functions that use
    string cws_dir; // the directory path of cws

public:
    Query() {
    }

    Query(const vector<int> & _seq, float _theta, int _k, string _cws_dir) :
        seqTokenized(_seq), theta(_theta), k(_k), cws_dir(_cws_dir){
        assert(theta <= 1.0);
    }

    vector<CW> getResult(unsigned int & winNum) {
        // Timer on
        auto timerOn = LogTime();

        vector<CW> res;
        vector<int> minHashesToken(k);      // save the tokenid that has minHashValue with each hashfunction (its length should be k)
        map<int, vector<CW>> doc_groups; // group CWs by their document id

        // get these K minHashes of the query sequence
        getKMinHash(minHashesToken);

        // Group those compat window vectors by T (document id)
        GroupT(doc_groups, minHashesToken);
        int filtered_groupNum = 0;
        // Implement Find Subset algorithm to each group and filter those whose size is lower than ceil(theta*k)
        int thres = int(ceil(k * theta));

        vector<map<int, vector<CW>>::iterator> its(doc_groups.size());
        int cnt=0;
        for(auto it =doc_groups.begin(); it!=doc_groups.end();it++){
            its[cnt++] = it;
        }

#pragma omp parallel for
        for (auto const & it  : its) {
            // filter each group's size
            if (it->second.size() < thres)
                continue;

            filtered_groupNum++;
            // Implement LineSweep Algorithm to find the intersection of intervals and get the result
            vector<CW> tmp_res;
            nearDupSearch(it->second, thres, res,winNum);
#pragma omp critical
            res.insert(res.end(), tmp_res.begin(), tmp_res.end());
        }

        printf("Filtered Groups Amount: %d\n", filtered_groupNum);
        printf("This query operation costs %f seconds\n", RepTime(timerOn));
        return res;
    }

private:
    void getKMinHash(vector<int> &minHashesToken) {
#pragma omp parallel for
        for (int i = 0; i < k; i++) {
            vector<int> hashValues(seqTokenized.size());

            for (int j = 0; j < seqTokenized.size(); j++) {
                hashValues[j] = hval(hashFunctions, seqTokenized[j], i);
            }
            // Get minHash of current hashfunction
            int minValuePos = min_element(hashValues.begin(), hashValues.end()) - hashValues.begin();
            minHashesToken[i] = seqTokenized[minValuePos];
        }
        printf("------------------MinHashesToken Generated------------------\n");
    }

    // Group those compat window vectors by T (document id)
    void GroupT(map<int, vector<CW>> &groups, const vector<int> &minHashesToken) {
        auto timerOn = LogTime();
        // load the coresponding cw vectors of these K minHashes
        // and group them by document idw
        int thres = int(ceil(k * theta));

        assert(minHashesToken.size() == k);

        vector<pair<IndexItem,int>> indexes(k);
        for (int i = 0; i < minHashesToken.size(); i++) {
            int token_id = minHashesToken[i];
            assert(token_id >= 0 && token_id < wordNum);
            indexes[i] = make_pair(indexArr[i][token_id], i);
            // vector<CW> cw_vet;

            // //load the corresponding cws file base on the current hash function
            // string cws_file = cws_dir + to_string(i) +".bin";
            // indexItem.getCompatWindows(cws_file, cw_vet, token_id);

            // cout<<"cws length "<<cw_vet.size()<<endl;
            // group them by their document id
            // for (auto &cw : cw_vet) {
            //     int doc_id = cw.T;

            //     if (groups.count(doc_id)) {
            //         groups[doc_id].push_back(cw);
            //     } else {
            //         vector<CW> &tmp_vet = groups[doc_id];
            //         tmp_vet.push_back(cw);
            //         assert(groups[doc_id].size() > 0); // will delete (just to check)
            //     }
            // }
        }

        sort(indexes.begin(),indexes.end());
        cout<<thres<<endl;
        for (int i = 0; i < thres; i++) {
            vector<CW> cw_vet;
            string cws_file = cws_dir + to_string(indexes[i].second) +".bin";
            indexes[i].first.getCompatWindows(cws_file, cw_vet);
            cout<<"cws length "<<cw_vet.size()<<endl;
        }

        printf("This GroupT operation costs %f seconds\n", RepTime(timerOn));
        printf("Groups Amount(Documents that minhashes corresponde): %lu\n", groups.size());
    }
};