#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <omp.h>

#include "ds/docIndex.hpp"
#include "ds/bigIndexItem.hpp"
#include "new_utils.hpp"
#include "utils.hpp"

#include "dupSearch/segmentTree.hpp"
#include "dupSearch/nearDupSearchFaster.hpp"
#include "ds/zoneMap.hpp"

using namespace std;

extern vector<pair<int, int>> hashFunctions;
extern BigIndexItem **indexArr;
extern DocIndex** docIndexArr;
extern int wordNum;
extern int docNum;
extern ZoneMaps zonemaps;
extern vector<SegmentTree> trees;

class QueryFaster {
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
    QueryFaster() {
    }

    QueryFaster(const vector<int> &_seq, float _theta, int _k, string _cws_dir, int _prefilter_size, string _t_dir_path, string _docOfs_dir_path):
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

        vector<pair<DocIndex,int>> doc_indexes(k);
        for (int i = 0; i < minHashesToken.size(); i++) {
            doc_indexes[i] = make_pair(docIndexArr[i][minHashesToken[i]], i);
        }
        sort(doc_indexes.begin(), doc_indexes.end());

        double getTCost = 0;

        // count how many tokens in each document under the sorted hash functions in the range of 0 to prefilter_size-1
        vector<vector<pair<int,unsigned>>> doc_tokens_count(docNum);
        unsigned primay_cands_amount = 0;
        int min_tokens_count = prefilter_size - (k - thres);
        printf("Min_tokens_count:%d\n", min_tokens_count);
        
        for (int i = 0; i < prefilter_size; i++) {
            auto timerOn = LogTime();

            // get the docsList
            auto ith_hashFun = doc_indexes[i].second;
            string t_file = t_dir_path + to_string(ith_hashFun) + ".bin";
            vector<int> t_vet;
            doc_indexes[i].first.getDocsList(t_file, t_vet);

            getTCost += RepTime(timerOn);

            // store the i and the t's position in t_vet into doc_tokens_count[t]
            for(unsigned j = 0;j<t_vet.size();j++){
                int t = t_vet[j];
                doc_tokens_count[t].emplace_back(i,j);
                if(doc_tokens_count[t].size()==min_tokens_count){
                    primay_cands_amount++;
                }
            }
        }
        
        IO_time += getTCost;
        cout << "Get T cost time: " << getTCost << endl;
        cout << "Enough tokens candidate text amount: " << primay_cands_amount << endl;
        cout << "Prefilter load T Got Cost: " << RepTime(timerOn) << endl;
        timerOn = LogTime();

        // get the documents that have enough tokens and their compact windows and store them into candidate texts(an unordered_map)
        for(int i =0; i<docNum;i++){
            if(doc_tokens_count[i].size()>=min_tokens_count){
                vector<CW> tmp_cws;
                for(auto const& p:doc_tokens_count[i]){
                    auto ith_hashFun = doc_indexes[p.first].second;
                    auto token_id = minHashesToken[ith_hashFun];
                    string ofs_file = docOfs_dir_path + to_string(ith_hashFun)+".bin";
                    auto specified_docOffset = doc_indexes[p.first].first.getSpecifiedDocOffset(ofs_file, p.second);

                    string cw_filePath = cws_dir + to_string(ith_hashFun) + ".bin";
                    indexArr[ith_hashFun][token_id].getOneDocumentCWs(cw_filePath, specified_docOffset, tmp_cws);
                }

                for(auto const & cw:tmp_cws){
                    groups[i].emplace_back(cw);
                    assert(cw.T == i);
                }
            }
        }

        printf("Primary Candidate Group Size: %u\n", groups.size());
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
            firstFileterNum++;

            // Implement LineSweep Algorithm to find the intersection of intervals
            vector<CW> tmp_res;
            unsigned tmp_winNum = 0;

            // nearDupSearch(it->second, tmp_thres, tmp_res, tmp_winNum);
            int thread_id = omp_get_thread_num();
            nearDupSearchFaster(it->second, min_tokens_count, tmp_res, trees[thread_id]);

#pragma omp critical
            if (tmp_res.size() != 0) {
                // cout<<"prefix filter: "<< doc_tokens_count[doc_id].size() <<endl;
                // printf("Now Candidate text %d has %ld cws\n",doc_id, groups[doc_id].size());
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
            int ith_khash = doc_indexes[i].second;
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
