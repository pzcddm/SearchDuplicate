#include <cstdio>
#include <cctype>
#include <iostream>
#include <fstream>
#include <execution>
#include <omp.h>
#include "util/utils.hpp"
#include "util/new_utils.hpp"

#include "util/cw.hpp"

int INTERVAL_LIMIT;

// Partition algorithm: In each recurrence, it will search and minimun element in current range and split the range into two pieces
void partition(const int &doc_id, const vector<int> &doc, const vector<int> &hashValues, int l, int r, vector<Wrapped_CW> &res_cws) {
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

    res_cws.emplace_back(doc[min_pos], doc_id, l, min_pos, r);
    partition(doc_id, doc, hashValues, l, min_pos - 1, res_cws);
    partition(doc_id, doc, hashValues, min_pos + 1, r, res_cws);
}

// get the compat windows of one document
void generateCompatWindow(const int &doc_id, const vector<int> &doc, vector<pair<int, int>> &hf, int ith_hf, vector<Wrapped_CW> &res_cws) {
    assert(INTERVAL_LIMIT >= 1);
    vector<int> hashValues(doc.size());
    for (int i = 0; i < doc.size(); i++) {
        hashValues[i] = hval(hf, doc[i], ith_hf);
    }

    partition(doc_id, doc, hashValues, 0, doc.size() - 1, res_cws);
}

// Todo: Build Index to memory
int main() {
    const string wiki_words_file_name = "wiki_test_words.txt";
    // const string src_path = "dataset/10k_byte/";
    // const string file_name ="10k.bytes";
    const string src_path = "dataset/test_byte/";
    const string file_name = "test.bytes";
    const string saved_dir = "compatWindows/test/";

    // string src_path = "./py_script/1k_dir/";
    // 先试试test文件夹里的文本

    int k = 40; // the number of hash functions
    // set the interval limit for generating compat windows
    INTERVAL_LIMIT = 16;

    //写死, 用seed为0~k-1的随机种子生成的k个hash function
    vector<pair<int, int>> hf;
    for (int i = 0; i < k; i++) generateHashFunc(i, hf);

    // Timer On
    auto start = LogTime();
    printf("------------------Loading Document File------------------\n");

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

    cout << "The interval limit when generating windows is " << INTERVAL_LIMIT << endl;

    unsigned long long total_cws_amount = 0;

    for (int i = 0; i < k; i++) {
        vector<Wrapped_CW> res_cws;
#pragma omp parallel for
        for (int doc_id = 0; doc_id < docs.size(); doc_id++) {
            vector<Wrapped_CW> tmp_vetor;

            generateCompatWindow(doc_id, docs[doc_id], hf, i, tmp_vetor);

#pragma omp critical
            res_cws.insert(res_cws.end(), tmp_vetor.begin(), tmp_vetor.end());
        }

        // sort the compat windows
        std::sort(std::execution::par_unseq, res_cws.begin(), res_cws.end());
        total_cws_amount += res_cws.size();

        // write these cws into a file
        string save_path = saved_dir + to_string(i) + ".bin";
        ofstream outFile(save_path, ios::out | ios::binary);

        for (auto const &wrapped_cw : res_cws) {
            assert(wrapped_cw.token_id >= 0);
            outFile.write((char *)&wrapped_cw, sizeof(wrapped_cw));
        }

        outFile.close();
        cout << save_path << " Saved\n";
    }

    cout << "total compat window amount: " << total_cws_amount << endl;

    printf("------------------Compat Windows Generated------------------\n");
    // Timer Off
    cout << "Compat Windows Generation, Sorting, and Saving Time Cost: " << RepTime(start) << " Seconds\n";

    cout << "sort complete and write cws into file" << endl;
}
