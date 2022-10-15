#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "util/nearDupSearch.hpp"
#include "util/cw.hpp"
#include "util/IO.hpp"

#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/indexItem.hpp"
#include "util/query.hpp"

using namespace std;

// global variables
IndexItem **indexArr;
vector<unordered_map<unsigned, int>> tokenId2index;
vector<vector<unsigned>> longTokenIds;
vector<vector<vector<pair<int, unsigned long long>>>> zoneMaps;
int zoneMpSize;

vector<pair<int, int>> hashFunctions;
int wordNum;
int docNum;

void prepareGlobalVariables(int k) {
    cout << "total token amount: " << wordNum << endl;
    // the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
    for (int i = 1; i <= k; i++) generateHashFunc(i, hashFunctions);
}

void loadZoneMap(int max_k, string zonemap_dir) {
    longTokenIds.resize(max_k);
    zoneMaps.resize(max_k);
    tokenId2index.resize(max_k);
    for (int i = 0; i < max_k; i++) {
        // open file
        string zonemapFile = zonemap_dir + to_string(i) + ".bin";
        ifstream inFile(zonemapFile, ios::in | ios::binary);

        longTokenIds[i].resize(zoneMpSize);
        zoneMaps[i].resize(zoneMpSize);

        for (int j = 0; j < zoneMpSize; j++) {
            unsigned tid;
            unsigned zp_len;

            inFile.read((char *)&tid, sizeof(unsigned));    // read token_id
            inFile.read((char *)&zp_len, sizeof(unsigned)); // read zonemap_length
            longTokenIds[i][j] = tid;
            assert(tokenId2index[i].count(tid) == 0);
            tokenId2index[i][tid] = j; // map the tid to its position in zoneMaps[i]
            zoneMaps[i][j].resize(zp_len);

            // load zoneMaps' textids and offsets
            for (int k = 0; k < zp_len; k++) {
                inFile.read((char *)&zoneMaps[i][j][k].first, sizeof(int));                 // read text_id
                inFile.read((char *)&zoneMaps[i][j][k].second, sizeof(unsigned long long)); // read offset
            }
        }
        inFile.close();
    }
}

void loadIndexItem(int k, string index_file) {
    printf("------------------Loading Index File------------------\n");
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::in | ios::binary);
    if (!inFile) {
        cout << "error open index file" << endl;
        return;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(IndexItem));
            if (indexArr[i][j].windowsNum < 0) {
                cout << i << " " << j << endl;
                cout << indexArr[i][j].windowsNum << " " << indexArr[i][j].offset << endl;
            }

            assert(indexArr[i][j].windowsNum >= 0);
        }
    }
    inFile.close();
    printf("------------------Index File Loaded------------------\n");
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
int main() {
    // Fixed parameters
    const string dataset = "openwebtext";
    const string tokSeqFile = "../gpt2output_64K_vocal/large-762M-k40.train.jsonl.bin";
    wordNum = 64000;   // the token amounts (vocabulary size)
    docNum = 8013769;  // the amount of texts
    zoneMpSize = 3000; // the size of zonemaps under one hashfunction
    const int T = 50;  // the T used in generating compact windows

    const int sample_sequence_num = 100;
    int max_k = 64;             // the maximum number of hash functions
    int k = 64;                 // the amount of hash functions intended to be used
    double prefix_length = 0.2; // control prefix length
    float theta = 0.9;          // similarity threshold
    int prefilter_size = int(ceil(0.2 * k) + k * prefix_length);

    // get the data path
    string cw_dir, indexFile, zonemap_dir;
    string root_dir = getRootDir(wordNum, max_k, T, docNum, zoneMpSize, dataset);
    getSonDir(root_dir, cw_dir, indexFile, zonemap_dir);

    // load the tokenized sequences
    vector<vector<int>> tokenizedSeqs;
    loadBin(tokSeqFile, tokenizedSeqs);

    prepareGlobalVariables(max_k);

    // load zone map
    loadZoneMap(max_k, zonemap_dir);

    // load the IndexItem
    loadIndexItem(max_k, indexFile);

    cout << "first tokenized seq Num" << tokenizedSeqs[0].size() << endl;

    // create random shuffle array to random sample the tokenizeSeq
    int *randomNum = new int[tokenizedSeqs.size()];
    for (int i = 0; i < tokenizedSeqs.size(); i++) {
        randomNum[i] = i;
    }
    random_shuffle(randomNum, randomNum + tokenizedSeqs.size());

    int find_num = 0;     // the num of those sequences have near dup
    int total_np_num = 0; // the num of finding np
    vector<int> find_np_arr;
    double total_query_time = 0;
    double total_IO_time = 0;
    // Create query
    int sample_times = sample_sequence_num;
    for (int i = 0; i < sample_times; i++) {
        auto const &seq = tokenizedSeqs[randomNum[i]];

        // make sure the sequence length is long enough
        if( seq.size()<k){
            sample_times++;
            cout<<"Meet seq, skip "<<endl;
            continue;
        }

        double query_time;
        unsigned int windowsNum = 0;
        Query query(seq, theta, k, cw_dir, prefilter_size);

        // Search near duplicate sentence
        vector<CW> duplicateCWs = query.getResult(windowsNum, query_time);
        total_IO_time += query.getIOtime();
        int np_passagesNum = reportPassagesNum(duplicateCWs);
        printf("Report Total Windows Num: %u\n", windowsNum);
        cout << "founded passages amount: " << np_passagesNum << endl;
        cout << "founded intervals amount: " << duplicateCWs.size() << endl;

        total_query_time += query_time;
        find_num += np_passagesNum ? 1 : 0;
        total_np_num += np_passagesNum;
        find_np_arr.emplace_back(np_passagesNum);
    }

    cout << "Sequence Query Over" << endl;
    printf(" memorized squences amount: %d  total_np_num: %d\n average query cost: %f average IO cost: ", find_num, total_np_num, total_query_time / sample_sequence_num, total_IO_time / sample_sequence_num);
}
