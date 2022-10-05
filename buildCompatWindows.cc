#include <cstdio>
#include <cctype>
#include <iostream>
#include <fstream>
#include <omp.h>
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

int INTERVAL_LIMIT;

// Partition algorithm: In each recurrence, it will search and minimun element in current range and split the range into two pieces
void partition(const int &doc_id, const vector<int> &doc, const int &ith_hf, const vector<int> &hashValues, int l, int r, vector<Wrapped_CW> &res_cws) {
    if (l + INTERVAL_LIMIT >= r)
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
    assert(INTERVAL_LIMIT >= 1);
    vector<int> hashValues(doc.size());
    for (int i = 0; i < doc.size(); i++) {
        hashValues[i] = hval(hf, doc[i], ith_hf);
    }

    partition(doc_id, doc, ith_hf, hashValues, 0, doc.size() - 1, res_cws);
}

// Todo: Build Index to memory
int main() {
    const string wiki_words_file_name = "wiki_test_words.txt";
    // const string src_path = "dataset/10k_byte/";
    // const string file_name ="10k.bytes";
    const string src_path = "dataset/test_byte/";
    const string file_name = "test.bytes";
    // string src_path = "./py_script/1k_dir/";
    // 先试试test文件夹里的文本

    int k = 100; // the number of hash functions
    // set the interval limit for generating compat windows
    INTERVAL_LIMIT = 20;

    //写死, 用seed为0~k-1的随机种子生成的k个hash function
    unsigned int seed = 0;
    vector<pair<int, int>> hf;
    for (int i = 0; i < k; i++) generateHashFunc(i, hf);

    // Timer On
    auto start = LogTime();
    printf("------------------Loading Document File------------------\n");

    // // 1. read stopwords from stopwords.txt
    // unordered_set<string> stopWords; // store stopwords
    // readStopWords("stopwords.txt", stopWords);
    // // 2. buildDic(src_path, stopWords, wiki_words_file_name);
    // //  Load the map of word to int
    // unordered_map<string, int> word2id;
    // loadWord2id(wiki_words_file_name, word2id);

    // // 3. read words from source folder
    // vector<string> files; // store file_path of each document in the given folder
    // getFiles(src_path, files);
    // // 4. load these files and change their words into token
    // start = chrono::high_resolution_clock::now();
    // int doc_num = files.size();
    // vector<vector<int>> docs(doc_num);
    // vector<vector<int>> docs_offset(doc_num);
    // cout << "current file size is: " << files.size() << endl;
    // int did = 0;
    // for (auto &file : files) {
    //     ProWords2Int(file, docs[did], docs_offset[did], word2id, stopWords);
    //     did++;
    // }

    vector<string> files; // store file_path of each document in the given folder
    vector<vector<int>> docs;
    vector<vector<int>> docs_offset;
    loadDocByBytes(file_name, src_path, docs, docs_offset, files);
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "readfile time: " << duration.count() / 1000000.0 << " seconds" << endl;
    int doc_num = files.size();
    cout << "doc number: " << doc_num << endl;

    printf("------------------Document File Loaded------------------\n");
    cout << "File Loaded Time Cost: " << RepTime(start) << " Seconds\n";

    // Timer On
    start = LogTime();

    printf("------------------Generating Compat Windows------------------\n");
    // generate compat windows for every document for every hash function
    vector<Wrapped_CW> res_cws;
    cout << "The interval limit when generating windows is " << INTERVAL_LIMIT << endl;
#pragma omp parallel for
    for (int doc_id = 0; doc_id < docs.size(); doc_id++) {
        vector<Wrapped_CW> tmp_vetor;
        for (int i = 0; i < k; i++) {
            generateCompatWindow(doc_id, docs[doc_id], hf, i, tmp_vetor);
        }
#pragma omp critical
        res_cws.insert(res_cws.end(), tmp_vetor.begin(), tmp_vetor.end());
    }

    cout << "total compat window amount: " << res_cws.size() << endl;

    sort(res_cws.begin(), res_cws.end());
    printf("------------------Compat Windows Generated------------------\n");
    // Timer Off
    cout << "Compat Windows Generation Time Cost: " << RepTime(start) << " Seconds\n";

    cout << "sort complete and write cws into file" << endl;
    // write these cws into a file
    ofstream outFile("CompatWindows", ios::out | ios::binary);
    unsigned long long compat_windows_num = 0;
    for (auto const &wrapped_cw : res_cws) {
        assert(wrapped_cw.token_id >=0);
        outFile.write((char *)&wrapped_cw, sizeof(wrapped_cw));
        compat_windows_num++;
    }
    unsigned long long tellp_pos = outFile.tellp();
    cout << "now the outFile tellg: "<<tellp_pos<<endl;
    cout<< "compat windows num"<< compat_windows_num<<endl;

    outFile.close();
}
