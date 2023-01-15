#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <omp.h>

#include "ds/indexItem.hpp"
#include "ds/bigIndexItem.hpp"
#include "new_utils.hpp"
#include "utils.hpp"
// #include "nearDupSearch.hpp"
#include "dupSearch/segmentTree.hpp"
#include "dupSearch/nearDupSearchFaster.hpp"
#include "ds/zoneMap.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern BigIndexItem **indexArr;
extern int wordNum;
extern int docNum;
extern ZoneMaps zonemaps;
// extern vector<unordered_map<unsigned, int>> tokenId2index;
// extern vector<vector<vector<pair<int, unsigned long long>>>> zoneMaps;
extern vector<SegmentTree> trees;
const int MAX_CWS_AMOUNT = 8e8;
const double MAX_IO_TIME = 600;

class Query {
private:
    float theta;        // threshold that is lower than 1
    int k;              // the num of hash functions that use
    string cws_dir;     // the directory path of cws
    int prefilter_size; // the size of smallest compat windows vectors loaded into prefilter
    short min_collide_requirement;

    vector<int> seqTokenized;
    vector<int> minHashesToken; // save the tokenid that has minHashValue with each hashfunction (its length should be k)

    double IO_time = 0;
    bool if_success = true;
public:
    Query() {
    }

    Query(const vector<int> &_seq, float _theta, int _k, string _cws_dir, double _prefilter_ratio) :
        seqTokenized(_seq), theta(_theta), k(_k), cws_dir(_cws_dir), prefilter_size(int(ceil(k*_prefilter_ratio))) {
        minHashesToken.resize(k);

        assert(theta <= 1.0);
        min_collide_requirement = int(ceil(k * theta));
    }

    double getIOtime() {
        return IO_time;
    }

    vector<CW> getResult(unsigned int &winNum, double &query_time) {
        // Timer on
        auto timerOn = LogTime();
        vector<CW> res;

        // get these K minHashes of the query sequence
        getKMinHash();

        // sort the indexes by their windows number
        vector<pair<BigIndexItem, int>> indexes = sortKIndexItems();

        // find candidate texts
        vector<pair<int, int>> candidate_texts;
        unordered_map<int, vector<CW>> doc_groups; // group CWs by their document id
        findCandTexts(indexes, doc_groups, candidate_texts);

        // Implement Find Subset algorithm to each group and filter those whose size is lower than ceil(theta*k)
        findOnceNearDup(indexes, doc_groups, candidate_texts, res);

        if(if_success== false){
            printf("It occurs too much cws so it fails! or the part of zonmap loading exceeds the time limit\n");
            return res;
        }

        query_time = RepTime(timerOn);
        printf("This query operation costs %f seconds\n", query_time);
        
        return res;
    }

private:
    // Get the K minhash of the query sequence
    void getKMinHash() {
#pragma omp parallel for
        for (int i = 0; i < k; i++) {
            // Calculate the hash values of the ith element sequence
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

    vector<pair<BigIndexItem, int>> sortKIndexItems() {
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

        return indexes;
    }

    // find the candidate texts
    void findCandTexts(const vector<pair<BigIndexItem, int>> &indexes, unordered_map<int, vector<CW>> &groups, vector<pair<int, int>> &candidate_texts) {
        printf("------------------Finding Candidate Texts------------------\n");
        // load the coresponding cw vectors of these prefilter_size minHashes
        // group them by document idw and then find candidate texts using dupSearch
        auto timerOn = LogTime();

        double getCwsCost = 0;
        unsigned long long prefilter_cws_amount = 0;

        int tmp_thres = prefilter_size - (k - min_collide_requirement);

        vector<int> groups_tokens(docNum);
        for (int i = 0; i < prefilter_size; i++) {
            auto timerOn = LogTime();
            vector<CW> cw_vet;
            string cws_file = cws_dir + to_string(indexes[i].second) + ".bin";
            indexes[i].first.getCompatWindows(cws_file, cw_vet);
            prefilter_cws_amount += cw_vet.size();
            getCwsCost += RepTime(timerOn);

            cout<<prefilter_cws_amount<<endl;
            // cout << "cws length " << cw_vet.size() << endl;

            if(prefilter_cws_amount > MAX_CWS_AMOUNT){
                if_success = false;
                return;
            }

            int pre_docId = -1;
            for (auto &cw : cw_vet) {
                int doc_id = cw.T;

                if (groups_tokens[doc_id] + prefilter_size - i < tmp_thres)
                    continue;

                groups[doc_id].emplace_back(cw);
                

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

            assert(doc_id < docNum);
            if (groups_tokens[doc_id] < tmp_thres)
                continue;
            firstFileterNum++;

            // Implement LineSweep Algorithm to find the intersection of intervals
            vector<CW> tmp_res;
            unsigned tmp_winNum = 0;

            // nearDupSearch(it->second, tmp_thres, tmp_res, tmp_winNum);
            int thread_id = omp_get_thread_num();
            auto collide_amount = nearDupSearchFaster(it->second, tmp_thres, tmp_res, trees[thread_id]);

#pragma omp critical
            if (tmp_res.size() != 0) {
                // cout<<"prefix filter: "<< groups_tokens[doc_id] <<endl;
                // printf("Now Candidate text %d has %d cws\n",doc_id, groups[doc_id].size());
                candidate_texts.emplace_back(collide_amount, doc_id);
            }
        }
        // cout << "firstFileterNum" << firstFileterNum << endl;

        // sort the candidate text pair so that the candidate texts with most collision will be ranked higher
        sort(candidate_texts.rbegin(), candidate_texts.rend());
        printf("candidate_texts amount: %lu\n", candidate_texts.size());

        cout << "Prefilter cal candidate text Got Cost: " << RepTime(timerOn) << endl;
        timerOn = LogTime();

        // // show out all the candidate texts
        // for (auto const & candidate : candidate_texts)
        //     cout << "one candidate text:" << candidate << endl;
    }

    // it will return if it find one nearDup
    void findOnceNearDup(const vector<pair<BigIndexItem, int>> &indexes, unordered_map<int, vector<CW>> &doc_groups, const vector<pair<int, int>> &candidate_texts, vector<CW> & res) {
        if(candidate_texts.size() == 0)
            return;
        
        if(if_success== false)
            return;

        auto timerOn = LogTime();

        // initialize infiles of those cws files
        vector<ifstream> inFiles(k);
        for (int i = prefilter_size; i < k; i++) {
            const auto &ith_khash = indexes[i].second;
            string cws_file = cws_dir + to_string(ith_khash) + ".bin";
            inFiles[i].open(cws_file, ios::in | ios::binary); // open file in a binary way
        }

        // iterate each candidate text and load those cws in them, then test neapDuplicates
        bool flag = false;
        auto max_preFilter_collide_amount = candidate_texts[0].first;

// #pragma omp parallel for
        for (auto const &candid_text : candidate_texts) {
            // todo : use doc tokens to validate if amount of tokens can reach the min_collide_requirement

            // if the flag is true and current candid_text.first is lower than max_preFilter_collide_amount
            // that means there could not be completely duplicate
            if(flag && candid_text.first<max_preFilter_collide_amount)
                break;
                
            vector<CW> text_cws;
            auto maxProbably_collide_amount = candid_text.first;

            // use zone map to load the rest of compact windows
            for (int i = prefilter_size; i < k; i++) {
                auto timerOn = LogTime();
                const auto &ith_khash = indexes[i].second;
                const auto& token_id = minHashesToken[ith_khash];
                zonemaps.getCWinText(inFiles[i], ith_khash, token_id, candid_text.second, text_cws);
                if (text_cws.size() == 0) {
                    continue;
                } else {
                    maxProbably_collide_amount++;
                }

                IO_time += RepTime(timerOn);

                if (text_cws.size() >= 50000) {
                    cout << ith_khash << " " << token_id << " " << candid_text.second << endl;
                }

                if(IO_time>MAX_IO_TIME){
                    if_success = false;
                    printf("Too much IO Time (exceed time limit %f) \n", MAX_IO_TIME);
                    return;
                }
                // Check if it is impossible to reach this requirement
                if (maxProbably_collide_amount + (k - i) < min_collide_requirement) {
                    break;
                }
            }

            if (maxProbably_collide_amount < min_collide_requirement) {
                continue;
            }

            // put these compact windows into groups
            for (auto const &cw : text_cws) {
                doc_groups[candid_text.second].emplace_back(cw);
            }

            // check their collision
            int thread_id = omp_get_thread_num();
            vector<CW> tmp_res;
            auto const &cw_vet = doc_groups[candid_text.second];
            nearDupSearchFaster(cw_vet, min_collide_requirement, tmp_res, trees[thread_id]);

            // if detect nearDuplicate
// #pragma omp critical
            {
                if(tmp_res.size())
                    res.insert(res.end(), tmp_res.begin(), tmp_res.end());
                if (res.size())
                    flag = true;
            }
        }

        // close the ifstreams
        for (auto &inFile : inFiles) {
            inFile.close();
        }

        printf("This zonemap load part and nearDup Search costs %f seconds\n", RepTime(timerOn));
        printf("Groups Amount(Documents that minhashes correspond): %lu\n", doc_groups.size());

        return;
    }
};
