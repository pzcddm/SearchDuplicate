#pragma once
#include <bits/stdc++.h>
#include <omp.h>

#include "../IO.hpp"
#include "../config/buildConfig.hpp"
#include "../new_utils.hpp"
#include "../utils.hpp"
#include "../ds/cw.hpp"
#include "../ds/bigIndexItem.hpp"
#include "cwGenerator.hpp"
using namespace std;

class IndexBuilder {
public:
    BuildConfig config;
    vector<pair<int, int>> hf;  // hash functions
    BigIndexItem **indexArr;       // Index Item
    vector<vector<int>> docs;   // the content of documents
    vector<vector<CW>> res_cws; // generated compact windows
    double writingDiskCost = 0; // time cost of writing files

private:
    void build_index_zonemap(const int ith_hash) {
        // Get the longest cws and their corresponding token_id
        vector<unsigned> cws_len(config.token_num);
        for (int j = 0; j < config.token_num; j++) {
            cws_len[j] = res_cws[j].size();
        }
        vector<unsigned> sorted_index = sort_index(cws_len);
        vector<unsigned> longest_cws_tid(config.zoneMpSize);
        int longestcws_cnt = 0;
        for (int i = config.token_num - config.zoneMpSize; i < config.token_num; i++) {
            longest_cws_tid[longestcws_cnt++] = sorted_index[i];
        }
        sort(longest_cws_tid.begin(), longest_cws_tid.end()); // let this token id ordered
        vector<vector<pair<int, unsigned long long>>> zonemap(config.zoneMpSize);
        int zonemp_cnt = 0;

        // write these cws into a file
        string save_path = config.cw_dirPath + to_string(ith_hash) + ".bin";
        ofstream outFile(save_path.c_str(), ios::out | ios::binary);

        unsigned long long offset = 0;
        for (int j = 0; j < config.token_num; j++) {
            if (j == longest_cws_tid[zonemp_cnt]) {
                // create zoneMap[zonemp_cnt]
                unsigned long long tmp_offset = offset;
                int stride = 0;
                int pre_text_id = -1;
                for (auto const &cw : res_cws[j]) {
                    if (stride == 0) {
                        if (pre_text_id != cw.T) { // that means current cw is the first cw of its text
                            zonemap[zonemp_cnt].emplace_back(cw.T, tmp_offset);
                            stride = config.zonemp_interval;
                        }
                    }
                    // update the pre cw's text id
                    pre_text_id = cw.T;
                    tmp_offset += sizeof(CW);
                    stride = max(0, stride - 1);
                }
                zonemp_cnt++;
            }

            for (auto const &cw : res_cws[j]) {
                outFile.write((char *)&cw, sizeof(cw));
            }
            indexArr[ith_hash][j].windowsNum = res_cws[j].size();
            indexArr[ith_hash][j].offset = offset;
            offset = offset + sizeof(CW) * indexArr[ith_hash][j].windowsNum;
        }
        outFile.close();

        // writing zonemap
        string zonemap_path = config.zoneMap_dirPath + to_string(ith_hash) + ".bin";

        cout << "Zone map writing" << zonemap_path << endl;
        ofstream zpFile(zonemap_path.c_str(), ios::out | ios::binary);

        for (int j = 0; j < longest_cws_tid.size(); j++) {
            unsigned tid = longest_cws_tid[j];
            unsigned zp_len = zonemap[j].size();

            zpFile.write((char *)&tid, sizeof(unsigned));    // write token_id
            zpFile.write((char *)&zp_len, sizeof(unsigned)); // write length of current zonemap

            for (auto const &pir : zonemap[j]) {
                zpFile.write((char *)&pir.first, sizeof(int));                 // write text_id
                zpFile.write((char *)&pir.second, sizeof(unsigned long long)); // write offset
            }
        }
        zpFile.close();
    }

    void write_index() {
        printf("------------------Writing Index File------------------\n");

        ofstream outFile(config.index_filePath, ios::out | ios::binary);
        for (int i = 0; i < config.k; i++) {
            for (int j = 0; j < config.token_num; j++) {
                outFile.write((char *)&indexArr[i][j], sizeof(BigIndexItem));
            }
        }
        outFile.close();
        printf("------------------Index File Writed------------------\n");
    }


public:
    // build the index
    void build() {
        auto start = LogTime();

        // load stopwords
        CwGenerator::filter.load_stopwords(config.stopwords_bin_path);
        
        // generate compat windows for every document for every hash function
        printf("------------------Generating Compat Windows------------------\n");

        unsigned long long total_cws_amount = 0;
        const int thread_num = omp_get_max_threads();
        vector<CwGenerator> generators(thread_num, CwGenerator(config.interval_limit));
        for (int i = 0; i < config.k; i++) {
            for (auto &generator : generators) generator.set_hf(hf[i]);

            // Three-dimensional(threads, tokens, compatwindows) arrays
            vector<vector<vector<CW>>> tmp_vetor(thread_num, vector<vector<CW>>(config.token_num));
#pragma omp parallel for // Genrate Compat windows under the current hash function
            for (int doc_id = 0; doc_id < config.doc_limit; doc_id++) {
                int tid = omp_get_thread_num();
                generators[tid].generate(doc_id, docs[doc_id], tmp_vetor[tid]);
            }

            // Merge inverted list generated from different threads and sort it
            res_cws.clear();
            res_cws.resize(config.token_num);
#pragma omp parallel for reduction(+ \
                                   : total_cws_amount)
            for (int j = 0; j < config.token_num; j++) {
                for (int tid = 0; tid < thread_num; tid++) {
                    res_cws[j].insert(res_cws[j].end(), tmp_vetor[tid][j].begin(), tmp_vetor[tid][j].end());
                    vector<CW>().swap(tmp_vetor[tid][j]);
                    // tmp_vetor[tid][j].clear();
                }
                // sort the compat windows
                sort(res_cws[j].begin(), res_cws[j].end());
                total_cws_amount += res_cws[j].size();
            }

            // build index and zonemap (also write the zonemap)
            auto writingDiskTimer = LogTime();
            build_index_zonemap(i);
            writingDiskCost += RepTime(writingDiskTimer);
        }

        // write index
        write_index();

        // check the stopwords should not generate any compact windows
        CwGenerator::filter.check_stopwords_index(indexArr, config);
        double total_time_cost = RepTime(start);
        cout << "total compat window amount: " << total_cws_amount << endl;
        printf("------------------Compat Windows Generated------------------\n");
        cout << "Compat Windows Generation, Sorting, and Saving Time Cost: " << total_time_cost << " Seconds\n"; // this time cost doesn't not include the time cost of loading bin files
        cout << "Disk Writing Time Cost: " << writingDiskCost << " Seconds\n";
        printf("Averaging over k disk IO time: %f  Computing time: %f compact windows amount: %f\n",
               writingDiskCost / config.k, (total_time_cost - writingDiskCost) / config.k, total_cws_amount * 1.0 / config.k);
    }

    IndexBuilder() {
    }

    IndexBuilder(BuildConfig _config) {
        config = _config;

        // the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
        for (auto seed = 1; seed <= config.k; seed++) generateHashFunc(seed, hf);

        // Allocate the memory for the index array
        indexArr = new BigIndexItem *[config.k];
        for (int i = 0; i < config.k; i++) {
            indexArr[i] = new BigIndexItem[config.token_num];
        }

        // Load the dataset/texts
        auto start = LogTime();
        printf("------------------Loading Document File------------------\n");
        loadBin(config.src_file, docs);
        cout << "readfile time: " << RepTime(start) << " seconds" << endl;
        // assert(config.doc_limit == docs.size());
        printf("------------------Document File Loaded------------------\n");
    }

    ~IndexBuilder() {
        for (int i = 0; i < config.k; i++) {
            delete[] indexArr[i];
        }
        delete[] indexArr;
    }
};