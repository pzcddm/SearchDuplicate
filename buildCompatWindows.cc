#include <cstdio>
#include <cctype>
#include <iostream>
#include <fstream>
#include <omp.h>
// #include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/IO.hpp"
#include "util/cw.hpp"
#include "util/indexItem.hpp"

int INTERVAL_LIMIT;

#define MAX_LENGTH 2000000

// Partition algorithm: In each recurrence, it will search and minimun element in current range and split the range into two pieces
void partition(const int &doc_id, const vector<int> &doc, const vector<pair<int, int>> &seg, int a, int b, vector<vector<CW>> &res_cws) {
    if (a + INTERVAL_LIMIT >= b)
        return;

    pair<int, int> ret(numeric_limits<int>::max(), -1);
    int n = doc.size();
    for (a += n, b += n; a <= b; ++a /= 2, --b /= 2) {
        if (a % 2 == 1)
            if (seg[a].first < ret.first)
                ret = seg[a];
        if (b % 2 == 0)
            if (seg[b].first < ret.first)
                ret = seg[b];
    }

    res_cws[doc[ret.second]].emplace_back(doc_id, a, ret.second, b);
    partition(doc_id, doc, seg, a, ret.second - 1, res_cws);
    partition(doc_id, doc, seg, ret.second + 1, b, res_cws);
}

// get the compat windows of one document
void generateCompatWindow(const int &doc_id, const vector<int> &doc, vector<pair<int, int>> &hf, int ith_hf, vector<vector<CW>> &res_cws, vector<pair<int, int>> &seg) {
    assert(INTERVAL_LIMIT >= 1);

    int n = doc.size();
    if (seg.size() < 2 * n)
        seg.resize(2 * n);

    for (int i = 0; i < n; i++) {
        seg[n + i].first = hval(hf, doc[i], ith_hf);
        seg[n + i].second = i;
    }

    for (int i = n - 1; i; i--) {
        if (seg[2 * i].first < seg[2 * i + 1].first)
            seg[i] = seg[2 * i];
        else
            seg[i] = seg[2 * i + 1];
    }

    partition(doc_id, doc, seg, 0, doc.size() - 1, res_cws);
}

// Todo: Build Index to memory
int main() {
    const string scr_dir = "../openwebtext_64K_vocal/";
    const string saved_dir = "compatWindows/openwebtext/";
    const string index_file = "index/indexOpenWebText.bin";

    const int tokenNum = 64000;
    int k = 100; // the number of hash functions
    // set the interval limit for generating compat windows
    INTERVAL_LIMIT = 50;

    //写死, 用seed为0~k-1的随机种子生成的k个hash function
    vector<pair<int, int>> hf;
    for (int i = 0; i < k; i++) generateHashFunc(i, hf);

    // Index Item
    IndexItem **indexArr;
    indexArr = new IndexItem *[k];
    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[tokenNum];
    }

    // Timer On
    auto start = LogTime();
    printf("------------------Loading Document File------------------\n");

    vector<vector<int>> docs;
    loadDataDir(scr_dir, docs);
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "readfile time: " << duration.count() / 1000000.0 << " seconds" << endl;
    int doc_num = docs.size();
    cout << "doc number: " << doc_num << endl;

    printf("------------------Document File Loaded------------------\n");
    cout << "File Loaded Time Cost: " << RepTime(start) << " Seconds\n";

    // Timer On
    start = LogTime();
    // time cost of writing files
    double writingDiskCost = 0;

    printf("------------------Generating Compat Windows------------------\n");
    // generate compat windows for every document for every hash function

    cout << "The interval limit when generating windows is " << INTERVAL_LIMIT << endl;

    unsigned long long total_cws_amount = 0;

    int thread_num = omp_get_max_threads();
    vector<vector<pair<int, int>>> segtrees(thread_num, vector<pair<int, int>>(MAX_LENGTH));

    for (int i = 0; i < k; i++) {
        vector<vector<vector<CW>>> tmp_vetor(thread_num, vector<vector<CW>>(tokenNum)); // Three-dimensional(threads, tokens, compatwindows) arrays

        // Genrate Compat windows under the current hash function
#pragma omp parallel for
        for (int doc_id = 0; doc_id < docs.size(); doc_id++) {
            int thread_id = omp_get_thread_num();
            generateCompatWindow(doc_id, docs[doc_id], hf, i, tmp_vetor[thread_id], segtrees[thread_id]);
        }

        // Merge inverted list generated from different threads and sort it
        vector<vector<CW>> res_cws(tokenNum);
#pragma omp parallel for reduction(+ \
                                   : total_cws_amount)
        for (int j = 0; j < tokenNum; j++) {
            for (int tid = 0; tid < thread_num; i++) {
                res_cws[j].insert(res_cws[j].end(), tmp_vetor[tid][j].begin(), tmp_vetor[tid][j].end());
            }
            // sort the compat windows]
            sort(res_cws[j].begin(), res_cws[j].end());
            total_cws_amount += res_cws.size();
        }

        // Timer ON
        auto writingDiskTimer = LogTime();

        // write these cws into a file
        string save_path = saved_dir + to_string(i) + ".bin";
        ofstream outFile(save_path, ios::out | ios::binary);

        unsigned long long offset = 0; 
        for (int j = 0; j < tokenNum; j++) {
            for (auto const &cw : res_cws[j]) {
                outFile.write((char *)&cw, sizeof(cw));
            }
            indexArr[i][j].windowsNum = res_cws[j].size();
            indexArr[i][j].offset = offset;
            offset = offset + sizeof(CW)*indexArr[i][j].windowsNum;
        }
        outFile.close();
        // Timer Off
        writingDiskCost += RepTime(writingDiskTimer);

        cout << save_path << " Saved\n";
        
    }

    cout << "total compat window amount: " << total_cws_amount << endl;
    printf("------------------Compat Windows Generated------------------\n");
    cout << "sort complete and write cws into file" << endl;
    // Timer Off

    cout << "Compat Windows Generation, Sorting, and Saving Time Cost: " << RepTime(start) << " Seconds\n";
    cout << "Disk Writing Time Cost: " << writingDiskCost << " Seconds\n";

    printf("------------------Writing Index File------------------\n");

    ofstream outFile(index_file, ios::out | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < tokenNum; j++) {
            outFile.write((char *)&indexArr[i][j], sizeof(IndexItem));
        }
    }
    outFile.close();
    printf("------------------Index File Writed------------------\n");
}
