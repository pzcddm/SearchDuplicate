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
vector<map<unsigned, int>> tokenId2index;
vector<vector<unsigned>> longTokenIds;
vector<vector<vector<pair<int, unsigned long long>>>> zoneMaps;
int zoneMpSize;

vector<pair<int, int>> hashFunctions;
int wordNum;
int docNum;

void prepareGlobalVariables(int k) {
    cout << "total token amount: " << wordNum << endl;
    // 写死, 用seed为0~k-1的随机种子生成的k个hash function
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
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(IndexItem));
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
    int max_k = 100;   // the maximum number of hash functions
    wordNum = 64000;   // the token amounts (vocabulary size)
    int k = 100;       // the amount of hash functions intended to be used
    docNum = 8013769;  // the amount of texts
    zoneMpSize = 4000; // the size of zonemaps under one hashfunction
    int prefilter_size = 30;

    float theta = 0.8;
    const string cw_dir = "compatWindows/openwebtext/";
    const string indexFile = "index/indexOpenWebText.bin";
    const string tokSeqFile = "../GenerationExperiment/tokenizeSeq/gpt2-small-seq.bin";
    const string zonemap_dir = "zonemap/openWebTextZP/";
    // load the tokenized sequences
    vector<vector<int>> tokenizedSeqs;
    loadBin(tokSeqFile, tokenizedSeqs);

    prepareGlobalVariables(max_k);

    // load zone map
    loadZoneMap(max_k, zonemap_dir);

    // load the IndexItem
    loadIndexItem(max_k, indexFile);

    cout << "tokenized seq Num" << tokenizedSeqs[0].size() << endl;

    if (tokenizedSeqs[0].size() < k) {
        cout << "error the tokenized sequence length is smaller than k!" << endl;
    }

    // Create query
    unsigned int windowsNum = 0;
    Query query(tokenizedSeqs[0], theta, k, cw_dir, prefilter_size);

    // Search near duplicate sentence
    vector<CW> duplicateCWs = query.getResult(windowsNum);

    printf("Report Total Windows Num: %u\n", windowsNum);
    cout << "total founded passages amount: " << reportPassagesNum(duplicateCWs) << endl;
    cout << "total founded intervals amount: " << duplicateCWs.size() << endl;
}