#include <cstdio>
#include <cctype>
#include <iostream>
#include <fstream>
#include "util/utils.hpp"
#include "util/new_utils.hpp"

#include "util/cw.hpp"

// save tokens
class TokenIndex {
    int k;
    int tokenNum;
    struct Index {
    };
};

// Partition algorithm: In each recurrence, it will search and minimun element in current range and split the range into two pieces
void partition(const int &doc_id, const vector<int> &doc, const int &ith_hf, const vector<int> &hashValues, int l, int r, vector<Wrapped_CW> &res_cws) {
    if (l > r)
        return;

    int minHash = hashValues[l];
    int min_pos = l;
    for (int i = l + 1; i <= r; i++) {
        if (minHash > hashValues[i]) {
            minHash = hashValues[i];
            min_pos = i;
        }
    }

    res_cws.emplace_back(doc[min_pos], ith_hf, doc_id, l, min_pos, r);
    partition(doc_id, doc, ith_hf, hashValues, l, min_pos - 1, res_cws);
    partition(doc_id, doc, ith_hf, hashValues, min_pos + 1, r, res_cws);
}

// get the compat windows of one document
void generateCompatWindow(const int &doc_id, const vector<int> &doc, vector<pair<int, int>> &hf, int ith_hf, vector<Wrapped_CW> &res_cws) {
    vector<int> hashValues(doc.size());
    int original_size = res_cws.size();
    for (int i = 0; i < doc.size(); i++) {
        hashValues[i] = hval(hf, doc[i], ith_hf);
    }

    partition(doc_id, doc, ith_hf, hashValues, 0, doc.size() - 1, res_cws);
    assert((res_cws.size() - original_size) == doc.size());
}

// Todo: Build Index to memory
int main() {
    const string wiki_words_file_name = "wiki_test_words.txt";

    // 先试试test文件夹里的文本
    string src_path = "./py_script/1k_dir/";
    int k = 10; // the number of hash functions

    // 1. read stopwords from stopwords.txt
    unordered_set<string> stopWords; // store stopwords
    readStopWords("stopwords.txt", stopWords);
    // 2. buildDic(src_path, stopWords, wiki_words_file_name);
    //  Load the map of word to int
    unordered_map<string, int> word2id;
    loadWord2id(wiki_words_file_name, word2id);
    // 3. 写死, 用seed为0的随机种子生成的k个hash function
    unsigned int seed = 0;
    vector<pair<int, int>> hf;
    for (int i = 0; i < k; i++) generateHashFunc(seed, hf);
    // 4. read words from source folder
    vector<string> files; // store file_path of each document in the given folder
    getFiles(src_path, files);
    // 5. load these files and change their words into token
    auto start = chrono::high_resolution_clock::now();
    int doc_num = files.size();
    vector<vector<int>> docs(doc_num);
    vector<vector<int>> docs_offset(doc_num);
    cout << "current file size is: " << files.size() << endl;
    int did = 0;
    for (auto &file : files) {
        ProWords2Int(file, docs[did], docs_offset[did], word2id, stopWords);
        did++;
    }
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "readfile time: " << duration.count() / 1000000.0 << " seconds" << endl;
    cout << "doc number: " << doc_num << endl;

    // generate compat windows for every document for every hash function

    vector<Wrapped_CW> res_cws;

    for (int doc_id = 0; doc_id < docs.size(); doc_id++) {
        for (int i = 0; i < k; i++) {
            generateCompatWindow(doc_id, docs[doc_id], hf, i, res_cws);
        }
    }

    cout << "total compat window amount: " << res_cws.size() << endl;

    sort(res_cws.begin(), res_cws.end());

    cout << "sort complete and write cws into file" << endl;

    // write these cws into a file
    ofstream outFile("CompatWindows", ios::out | ios::binary);
    for (auto const &wrapped_cw : res_cws) {
        outFile.write((char *)&wrapped_cw, sizeof(wrapped_cw));
    }
    outFile.close();
}