#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <omp.h>

#include "docIndex.hpp"
#include "bigIndexItem.hpp"
#include "new_utils.hpp"
#include "utils.hpp"

#include "dupSearch/segmentTree.hpp"
#include "dupSearch/nearDupSearchFaster.hpp"
#include "zoneMap.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern BigIndexItem **indexArr;
extern DocIndex** docIndexArr;
extern int wordNum;
extern int docNum;
extern ZoneMaps zonemaps;
extern vector<SegmentTree> trees;

class Query {
    vector<int> seqTokenized;
    float theta;        // threshold that is lower than 1
    int k;              // the num of hash functions that use
    string cws_dir;     // the directory path of cws
    string t_dir_path; // the directory path of t 
    string docOfs_dir_path;  // the directory path of docs_offset
    int prefilter_size; // the size of smallest compat windows vectors loaded into prefilter
private:
    double IO_time = 0;

public:
    Query() {
    }

    Query(const vector<int> &_seq, float _theta, int _k, string _cws_dir, int _prefilter_size, string _t_dir_path, string _docOfs_dir_path):
        seqTokenized(_seq), theta(_theta), k(_k), cws_dir(_cws_dir), prefilter_size(_prefilter_size), t_dir_path(_t_dir_path), docOfs_dir_path(_docOfs_dir_path) {
        assert(theta <= 1.0);
    }

    double getIOtime() {
        return IO_time;
    }

    vector<CW> getResult(unsigned int &winNum, double &query_time) {    
        // Timer on
        auto timerOn = LogTime();

        vector<CW> res;
        vector<int> minHashesToken(k);             // save the tokenid that has minHashValue with each hashfunction (its length should be k)
        unordered_map<int, vector<CW>> doc_groups; // group CWs by their document id

        // get these K minHashes of the query sequence
        getKMinHash(minHashesToken);

        // Group those compat window vectors by T (document id)
        vector<int> candidate_texts;
        GroupT(doc_groups, minHashesToken, candidate_texts);
        int filtered_groupNum = 0;
        // Implement Find Subset algorithm to each group and filter those whose size is lower than ceil(theta*k)
        int thres = int(ceil(k * theta));

        vector<unordered_map<int, vector<CW>>::iterator> its(doc_groups.size());
        int cnt = 0;
        for (auto it = doc_groups.begin(); it != doc_groups.end(); it++) {
            its[cnt++] = it;
        }

        int flag = false;
#pragma omp parallel for
        for (const auto &candid_tid : candidate_texts) {
            if (flag) { // need to be delted just to find if there is near duplicate
                continue;
            }
            // filter each group's size
            auto const &cw_vet = doc_groups[candid_tid];
            if (cw_vet.size() < thres)
                continue;
            else
                cout << candid_tid <<" has cws :" <<cw_vet.size()<<endl;

            // // output all the compact windows
            // for (const auto &cw: cw_vet){
            //     cw.display();
            // }
            filtered_groupNum++;
            // Implement LineSweep Algorithm to find the intersection of intervals and get the result
            vector<CW> tmp_res;
            // nearDupSearch(cw_vet, thres, tmp_res, winNum);
            int thread_id = omp_get_thread_num();
            nearDupSearchFaster(cw_vet, thres, tmp_res, trees[thread_id]);

#pragma omp critical
            {
                if (res.size())
                    flag = true;
                res.insert(res.end(), tmp_res.begin(), tmp_res.end());
            }
        }

        // printf("Filtered Groups Amount: %d\n", filtered_groupNum);
        query_time = RepTime(timerOn);
        printf("This query operation costs %f seconds\n", query_time);
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
            assert(minHashesToken[i] >= 0);
        }
        printf("------------------MinHashesToken Generated------------------\n");
    }

    // Group those compat window vectors by T (document id)
    void GroupT(unordered_map<int, vector<CW>> &groups, const vector<int> &minHashesToken, vector<int> &candidate_texts) {
        auto timerOn = LogTime();
        // load the coresponding cw vectors of these K minHashes
        // and group them by document idw
        int thres = int(ceil(k * theta));

        assert(minHashesToken.size() == k);

        vector<pair<BigIndexItem, int>> indexes(k);
        for (int i = 0; i < minHashesToken.size(); i++) {
            int token_id = minHashesToken[i];
            if (token_id < 0 || token_id >= wordNum) {
                cout << "Error! token id Error" << token_id << endl;
            }
            assert(token_id >= 0 && token_id < wordNum);
            indexes[i] = make_pair(indexArr[i][token_id], i);
        }

        sort(indexes.begin(), indexes.end());
        vector<int> groups_tokens(docNum);

        double getCwsCost = 0;
        unsigned long long prefilter_cws_amount = 0;
        int tmp_thres = prefilter_size - (k - thres);
        for (int i = 0; i < prefilter_size; i++) {
            auto timerOn = LogTime();
            vector<CW> cw_vet;
            string cws_file = cws_dir + to_string(indexes[i].second) + ".bin";
            indexes[i].first.getCompatWindows(cws_file, cw_vet);
            prefilter_cws_amount += cw_vet.size();
            getCwsCost += RepTime(timerOn);
            // cout << "cws length " << cw_vet.size() << endl;

            int pre_docId = -1;
            for (auto &cw : cw_vet) {
                int doc_id = cw.T;

                if (groups_tokens[doc_id] + prefilter_size - i < tmp_thres)
                    continue;

                groups[doc_id].emplace_back(cw);;

                if (pre_docId != doc_id) {
                    groups_tokens[doc_id]++;
                    pre_docId = doc_id;
                }
            }
        }

        IO_time += getCwsCost;
        cout << "prefilter_cws_amount: " << prefilter_cws_amount << endl;
        cout << "Get Cw cost time: " << getCwsCost << endl;
        cout << "Prefilter load cw Got Cost: " << RepTime(timerOn) << endl;
        cout << "Current Groups amount: " << groups.size() << endl;
        timerOn = LogTime();

        // get candidate texts
        vector<unordered_map<int, vector<CW>>::iterator> its(groups.size());
        int cnt = 0;
        for (auto it = groups.begin(); it != groups.end(); it++) {
            its[cnt++] = it;
        }

        int firstFileterNum = 0; // indicate how many times the linesweep algorithm will be used in prefilter
                                 // cout<< "tmp_thres"<<tmp_thres<<endl;
#pragma omp parallel for
        for (auto const &it : its) {
            // filter each group's size
            int doc_id = it->first;
            
            assert(doc_id<docNum);
            if (groups_tokens[doc_id] < tmp_thres)
                continue;
            firstFileterNum++;

            // Implement LineSweep Algorithm to find the intersection of intervals
            vector<CW> tmp_res;
            unsigned tmp_winNum = 0;

            // nearDupSearch(it->second, tmp_thres, tmp_res, tmp_winNum);
            int thread_id = omp_get_thread_num();
            nearDupSearchFaster(it->second, tmp_thres, tmp_res, trees[thread_id]);

#pragma omp critical
            if (tmp_res.size() != 0) {
                cout<<"prefix filter: "<< groups_tokens[doc_id] <<endl;
                printf("Now Candidate text %d has %d cws\n",doc_id, groups[doc_id].size());
                candidate_texts.emplace_back(doc_id);
            }
        }
        // cout << "firstFileterNum" << firstFileterNum << endl;
        printf("candidate_texts amount: %lu\n", candidate_texts.size());

        cout << "Prefilter cal candidate text Got Cost: " << RepTime(timerOn) << endl;
        timerOn = LogTime();
        
        // show out all the candidate texts
        for (auto const & candidate : candidate_texts)
            cout << "one candidate text:" << candidate << endl;
        
        // iterate the left indexs and load those cws in candidates texts
        for (int i = prefilter_size; i < k; i++) {
            int ith_khash = indexes[i].second;
            int token_id = minHashesToken[ith_khash];
            string cws_file = cws_dir + to_string(ith_khash) + ".bin";
            ifstream inFile(cws_file, ios::in | ios::binary); //二进制读方式打开
            if (!inFile) {
                cout << "error open file" << endl;
                return;
            }
            for (auto const &candid_text : candidate_texts) {
                // use zone map

                auto timerOn = LogTime();

                vector<CW> text_cws;
                zonemaps.getCWinText(inFile, ith_khash, token_id, candid_text, text_cws);
                if (text_cws.size() == 0) {
                    continue;
                }
        
                IO_time += RepTime(timerOn);
                assert(text_cws.size() < 50000); // the amount of compact windows in one text of one token normally is  low (lower than 1e4)
                // if(text_cws.size()>0){
                //     cout<<"text_cws size"<<text_cws.size()<<endl;
                // }
                assert(groups.count(candid_text));

                // put these compact windows into groups
                for (auto const &cw : text_cws) {
                    groups[candid_text].emplace_back(cw);
                }
            }
            inFile.close();
        }
        printf("This zonemap found part costs %f seconds\n", RepTime(timerOn));
        printf("Groups Amount(Documents that minhashes corresponde): %lu\n", groups.size());
    }
};
