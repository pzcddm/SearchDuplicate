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
    // Specify the dataset
    const string wiki_words_file_name = "wiki_train_words.txt";
    const string src_path = "dataset/train_byte/";
    const string file_name = "train.bytes";
    const string output_file = "CompatWindowsTrainRaw.bin";

    int k = 30; // the number of hash functions
    // set the interval limit for generating compat windows
    INTERVAL_LIMIT = 10;

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

    // Prepare the outFile
    ofstream outFile(output_file, ios::out | ios::binary);

    // count the amount of compat_windows_num
    unsigned long long compat_windows_num = 0;

    // Timer On
    start = LogTime();

    printf("------------------Generating Compat Windows------------------\n");
    // generate compat windows for every document for every hash function
    cout << "The interval limit when generating windows is " << INTERVAL_LIMIT << endl;
#pragma omp parallel for
    for (int doc_id = 0; doc_id < docs.size(); doc_id++) {
        vector<Wrapped_CW> tmp_vetor;
        for (int i = 0; i < k; i++) {
            generateCompatWindow(doc_id, docs[doc_id], hf, i, tmp_vetor);
        }

        // write the tmp_vetor to the outFile (you cannot save it in the memory because memory is not big enough)
#pragma omp critical
        {
            for (auto const &wrapped_cw : tmp_vetor) {
                assert(wrapped_cw.token_id >= 0);
                outFile.write((char *)&wrapped_cw, sizeof(wrapped_cw));
            }
            compat_windows_num += tmp_vetor.size();
        }
    }

    cout << "total compat window amount: " << compat_windows_num << endl;

    printf("------------------Compat Windows Generated ------------------\n");

    // Timer Off
    cout << "Compat Windows Generation Time Cost: " << RepTime(start) << " Seconds\n";

    cout << "write unsorted cws into file" << endl;
    unsigned long long tellp_pos = outFile.tellp();
    cout << "now the outFile tellg: " << tellp_pos << endl;
    outFile.close();
}
