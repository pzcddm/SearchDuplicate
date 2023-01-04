#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

// #include "util/nearDupSearch.hpp"
#include "util/ds/cw.hpp"
#include "util/ds/docIndex.hpp"
#include "util/IO.hpp"

#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/ds/bigIndexItem.hpp"
#include "util/query.hpp"
// #include "util/queryFaster.hpp"
#include "util/dupSearch/segmentTree.hpp"
using namespace std;

// global variables
DocIndex** docIndexArr;
BigIndexItem **indexArr;
vector<SegmentTree> trees;
ZoneMaps zonemaps;
vector<pair<int, int>> hashFunctions;

int wordNum;
int docNum;

void prepareGlobalVariables(int k) {
    cout << "total token amount: " << wordNum << endl;
    // the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
    for (int i = 1; i <= k; i++) generateHashFunc(i, hashFunctions);

    // intiliaze thread_num segment tree for parelled near duplicate search
    int thread_num = omp_get_max_threads();
    trees.resize(thread_num);
}

int reportPassagesNum(const vector<CW> &duplicateCWs) {
    int pasNum = 0;
    CW tmp_cw(0, -1, -1, -1);
    for (auto const &cw : duplicateCWs) {
        if (tmp_cw.intersected(cw)) {
            tmp_cw.merge(cw);
        } else {
            pasNum++;
            tmp_cw = cw;
        }
    }
    return pasNum;
}

void display_parameters(const int &tokenNum, const int &k, const int &T, const float &theta, const int &zoneMpSize, const int &fixed_prefixe, const int &sample_sequence) {
    printf("tokenNum: %d ,k: %d , T:%d , theta:%f, zoneMpSize: %d fixed_prefix: %d total_sample_sequence %d \n", tokenNum, k, T, theta, zoneMpSize, fixed_prefixe, sample_sequence);
} 

int main(int argc, char **argv) {
    // Fixed parameters
    string source_bin_file = "../dataset_tokenizedGbt2/openwebtext_gpt2.bin";
    string dataset = "pile";
    // string dataset = "openwebtext";
    // string tokSeqFile = "../SelfGenerationText/gpt2-medium-540L_50TOPK_400000S.bin";
    // string tokSeqFile = "./openwebtext_sampled_docs.bin";
    string tokSeqFile = "../SelfGenerationText/gpt-neo-540L_50TOPK_1_3B.bin";
    // string tokSeqFile = "./pile_sampled_docs.bin";

    wordNum = 50257;
    docNum = 210607728; // the amount of texts in the dataset 210607728 8013769
    // int zoneMpSize = 8000; // the size of zonemaps under one hashfunction
    int zoneMpSize = 50257;  //8000 50257
    int T = 50;            // the T used in generating compact windows
    int fixed_prefix = 64; // or 128

    bool if_showPassage = false;
    int sample_sequence_num = 1000; 
    int sample_start = 0;
    int max_windows_num = 10000;
    int max_k = 64;             // the maximum number of hash functions
    int k = 64;                 // the amount of hash functions intended to be used
    double prefix_length = 0.4; // control prefix length
    float theta = 0.8;          // similarity threshold 
    int prefilter_size = int(ceil(k * prefix_length)); 

    // load document index 

    string doc_index_file = "./doc_index/openwebtext_gpt2_docIndex.bin";
    vector<unsigned long long> doc_index;
    readDocInex(doc_index, doc_index_file);

    // load parameters
    for (int i = 0; i < argc; i++) { 
        string arg = argv[i];
        if (arg == "-dataset") {
            dataset = string(argv[i + 1]);
        }
        if (arg == "-fixed_prefix") {
            fixed_prefix = atoi(argv[i + 1]);
        }
        if (arg == "-tokSeqFile") {
            tokSeqFile = string(argv[i + 1]);
        }
        if (arg == "-wordNum") {
            wordNum = atoi(argv[i + 1]);
        }
        if (arg == "-docNum") {
            docNum = atoi(argv[i + 1]);
        }
        if (arg == "-zoneMpSize") {
            zoneMpSize = atoi(argv[i + 1]);
        }
        if (arg == "-T") {
            T = atoi(argv[i + 1]);
        }
        if (arg == "-sample_sequence_num") {
            sample_sequence_num = atoi(argv[i + 1]);
        }
        if (arg == "-k") { 
            k = atoi(argv[i + 1]);
        }
        if (arg == "-prefix_length") {
            prefix_length = stod(string(argv[i + 1]));
        }
        if (arg == "-theta") {
            theta = atof(argv[i + 1]);
        }
    }

    cout << tokSeqFile << endl;
    display_parameters(wordNum, k, T, theta, zoneMpSize, fixed_prefix, sample_sequence_num);
    // get the data path
    string cw_dir, indexFile, zonemap_dir;
    string root_dir = getRootDir(wordNum, max_k, T, docNum, zoneMpSize, dataset);
    getSonDir(root_dir, cw_dir, indexFile, zonemap_dir);

    // load the tokenized sequences
    vector<vector<int>> tokenizedSeqs;
    loadBin(tokSeqFile, tokenizedSeqs);

    prepareGlobalVariables(max_k);

    // Initialize and load zone map
    zonemaps = ZoneMaps(max_k, zoneMpSize);
    zonemaps.load(zonemap_dir);
    // loadZoneMap(max_k, zonemap_dir);

    // load the IndexItem
    loadBigIndexItem(indexArr, wordNum, max_k, indexFile);

    // load the docIndex
    string docIndex_file_path = "./index/pile_50K_64k_50T_210M_50257ZP/docIndex.bin";
    loadDocIndex(docIndexArr, wordNum, max_k, docIndex_file_path);

    string t_dir_path = "./index/pile_50K_64k_50T_210M_50257ZP/t/";
    string docOfs_dir_path = "./index/pile_50K_64k_50T_210M_50257ZP/docOfs/";

    cout << "first tokenized seq Num" << tokenizedSeqs[0].size() << endl;

    // create random shuffle array to random sample the tokenizeSeq
    int *randomNum = new int[tokenizedSeqs.size()];
    for (int i = 0; i < tokenizedSeqs.size(); i++) {
        randomNum[i] = i;
    }

    int find_num = 0;     // the num of those sequences have near dup
    int total_np_num = 0; // the num of finding np
    vector<int> find_np_arr;
    double total_query_time = 0;
    double total_IO_time = 0;
    map<int, int> mp;
    map<double, int> theta_mp;
    theta_mp[0.8] = 0;
    theta_mp[0.9] = 0;
    theta_mp[1.0] = 0;

    // Create query
    int sample_times = sample_sequence_num;

    int token_len_thres = max(k, fixed_prefix);

    int windows_num = 0;
    int traversed_sequences_num = 0;
    for (int i = 0; i < sample_times; i++) {
        auto &raw_seq = tokenizedSeqs[randomNum[i + sample_start]];

        // make sure the sequence length is long enough
        if (raw_seq.size() < token_len_thres) {
            sample_times++;
            cout << "Meet short seq, skip " << endl;
            continue;
        }
        cout << "New Sequence length: " << raw_seq.size() << endl;
        // Intercept prefix
        cout << "current sequences no: " << i + sample_start << endl;
        cout << "current traversed sequences number: " << traversed_sequences_num << endl;
        cout << "windows current: " << windows_num << endl;
        traversed_sequences_num++;
        for (int j = 0; j + fixed_prefix <= raw_seq.size(); j += fixed_prefix) {
            system("keep-job 48");
            windows_num++;
            vector<int> seq;
            seq.assign(raw_seq.begin() + j, raw_seq.begin() + fixed_prefix + j);
            double query_time;
            unsigned int cwNum = 0;
            Query query(seq, theta, k, cw_dir, prefilter_size);
            // QueryFaster query(seq, theta, k, cw_dir, prefilter_size, t_dir_path, docOfs_dir_path);
            // Search near duplicate sentence
            vector<CW> duplicateCWs = query.getResult(cwNum, query_time);
            total_IO_time += query.getIOtime();

            bool flags[3] = {false, false, false};
            for (auto const &cw : duplicateCWs) {
                double tmp_theta = int(cw.c * 1.0 / k * 10.0) / 10.0;
                assert(tmp_theta >= 0.8);
                if (flags[int(ceil((tmp_theta - 0.8) * 10))] == false) {
                    flags[int(ceil((tmp_theta - 0.8) * 10))] = true;
                }
            }

            // output the duplicate passage
            if (if_showPassage && duplicateCWs.size()) {
                int max_id;
                int max_length = 0;
                for (int m = 0; m < duplicateCWs.size(); m++) {
                    auto tmp_length = duplicateCWs[m].r - duplicateCWs[m].l;
                    if (tmp_length >= max_length) {
                        max_id = m;
                        max_length = tmp_length;
                    }
                }
                cout << "Print one of the duplicate passage:\n";
                vector<int> dup_passage;
                auto cw = duplicateCWs[max_id];
                getPassage(cw.T, doc_index, source_bin_file, cw.l, cw.r, dup_passage);
                printVec(dup_passage);
            }

            if (flags[2] == true) {
                flags[1] = true;
                flags[0] = true;
            }

            if (flags[1] == true) {
                flags[0] = true;
            }
            theta_mp[0.8] += flags[0];
            theta_mp[0.9] += flags[1];
            theta_mp[1.0] += flags[2];

            // Extract differtent theta from different
            int np_passagesNum = reportPassagesNum(duplicateCWs);
            if (np_passagesNum > 0) {
                find_num++;

                if (if_showPassage) {
                    cout << "Show the query seq:\n";
                    printVec(seq);
                    printf("found a near duplicate window current near duplicate:%d\n ", find_num);
                }

                mp[np_passagesNum]++;
                total_np_num += np_passagesNum;
                find_np_arr.emplace_back(np_passagesNum);

                for (auto const &it : theta_mp) {
                    printf("theta:%f passage_num:%d\n", it.first, it.second);
                }
            }
            total_query_time += query_time;

            if (windows_num >= max_windows_num)
                break;
        }
        if (windows_num >= max_windows_num)
            break;
    }

    for (auto const &it : mp) {
        printf("np: %d value: %d\n", it.first, it.second);
    }

    for (auto const &it : theta_mp) {
        printf("theta:%f passage_num:%d\n", it.first, it.second);
    }

    cout << tokSeqFile << endl;
    cout << traversed_sequences_num << " Sequences Query Over" << endl;
    cout << "windows Num: " << windows_num << endl;
    display_parameters(wordNum, k, T, theta, zoneMpSize, fixed_prefix, traversed_sequences_num);
    printf("total sequence num: %d total windows num: %d , near duplicate windows num %d\n", traversed_sequences_num, windows_num, find_num);
    printf(" memorized squences amount: %d  total_np_num: %d\n average query cost: %f average IO cost: %f, average caculation cost: %f", find_num, total_np_num, total_query_time / max_windows_num, total_IO_time / max_windows_num, (total_query_time - total_IO_time) / max_windows_num);
}
