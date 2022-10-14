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
void partition(const int &doc_id, const vector<int> &doc, const vector<pair<int, int>> &seg, int l, int r, vector<vector<CW>> &res_cws) {
    if (l + INTERVAL_LIMIT >= r)
        return;

    pair<int, int> ret(numeric_limits<int>::max(), -1);
    int n = doc.size();
    int a = l, b = r;
    for (a += n, b += n; a <= b; ++a /= 2, --b /= 2) {
        if (a % 2 == 1)
            if (seg[a].first < ret.first)
                ret = seg[a];
        if (b % 2 == 0)
            if (seg[b].first < ret.first)
                ret = seg[b];
    }

    assert(doc[ret.second] >= 0 && doc[ret.second] < 64000);
    res_cws[doc[ret.second]].emplace_back(doc_id, l, ret.second, r);
    partition(doc_id, doc, seg, l, ret.second - 1, res_cws);
    partition(doc_id, doc, seg, ret.second + 1, r, res_cws);
}

// get the compat windows of one document
void generateCompatWindow(const int &doc_id, const vector<int> &doc, vector<pair<int, int>> &hf, int ith_hf, vector<vector<CW>> &res_cws, vector<pair<int, int>> &seg) {
    assert(INTERVAL_LIMIT >= 1);

    int n = doc.size();
    if (seg.size() < 2 * n) {
        seg.resize(2 * n);
        cout << seg.size() << endl;
    }

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
    const string saved_dir = "compatWindows/openwebtext_64K_1000Gap/";
    const string index_file = "index/indexOpenWebText.bin";
    const string zoneMap_dir = "zonemap/openWebTextZP_1000Gap_3000Size/";

    const int tokenNum = 64000;
    int k = 100;                       // the number of hash functions
    INTERVAL_LIMIT = 50;               // set the interval limit for generating compat windows
    const int zonemp_interval = 1000;  // the stride that decreasing when generating zonemap
    const int zoneMpSize = 3000;       // the size of zonemaps under one hashfunction

    //the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
    vector<pair<int, int>> hf;
    for (int i = 1; i <= k; i++) generateHashFunc(i, hf);

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

        cout << "hello" << endl;
        // Merge inverted list generated from different threads and sort it
        vector<vector<CW>> res_cws(tokenNum);
#pragma omp parallel for reduction(+ \
                                   : total_cws_amount)
        for (int j = 0; j < tokenNum; j++) {
            for (int tid = 0; tid < thread_num; tid++) {
                res_cws[j].insert(res_cws[j].end(), tmp_vetor[tid][j].begin(), tmp_vetor[tid][j].end());
                vector<CW>().swap(tmp_vetor[tid][j]);
                // tmp_vetor[tid][j].clear();
            }
            // sort the compat windows
            sort(res_cws[j].begin(), res_cws[j].end());
            total_cws_amount += res_cws[j].size();
        }
        cout << "Current cws amount:" << total_cws_amount << endl;

        // Get the longest cws and their corresponding token_id
        vector<unsigned> cws_len(tokenNum);
        for (int j = 0; j < tokenNum; j++) {
            cws_len[j] = res_cws[j].size();
        }
        vector<unsigned> sorted_index = sort_index(cws_len);
        vector<unsigned> longest_cws_tid(zoneMpSize);
        int longestcws_cnt =0;
        for (int i = tokenNum - zoneMpSize ; i < tokenNum; i++) {
            longest_cws_tid[longestcws_cnt++] = sorted_index[i];
        }
        sort(longest_cws_tid.begin(), longest_cws_tid.end()); // let this token id ordered
        vector<vector<pair<int, unsigned long long>>> zonemap(zoneMpSize);
        int zonemp_cnt = 0;

        // Timer ON
        auto writingDiskTimer = LogTime();

        // write these cws into a file
        string save_path = saved_dir + to_string(i) + ".bin";
        ofstream outFile(save_path, ios::out | ios::binary);

        unsigned long long offset = 0;
        for (int j = 0; j < tokenNum; j++) {
            if (j == longest_cws_tid[zonemp_cnt]) {
                // create zoneMap[zonemp_cnt]
                unsigned long long tmp_offset = offset;
                int stride = 0;
                int pre_text_id = -1;
                for (auto const &cw : res_cws[j]) {
                    if (stride == 0) {
                        if (pre_text_id != cw.T) { // that means current cw is the first cw of its text
                            zonemap[zonemp_cnt].emplace_back(cw.T, tmp_offset);
                            stride = zonemp_interval;
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
            indexArr[i][j].windowsNum = res_cws[j].size();
            indexArr[i][j].offset = offset;
            offset = offset + sizeof(CW) * indexArr[i][j].windowsNum;
        }
        outFile.close();

        // writing zonemap
        string zonemap_path = zoneMap_dir + to_string(i) + ".bin";
        ofstream zpFile(zonemap_path, ios::out | ios::binary);

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

        // Timer Off
        writingDiskCost += RepTime(writingDiskTimer);

        cout << save_path << " Saved\n";
        cout << zonemap_path << " Saved\n";
    }

    cout << "total compat window amount: " << total_cws_amount << endl;
    printf("------------------Compat Windows Generated------------------\n");
    cout << "sort complete and write cws into file" << endl;

    // Timer Off
    double total_time_cost = RepTime(start);
    cout << "Setting Hash functions Amount: "<<k<<endl;
    
    cout << "Compat Windows Generation, Sorting, and Saving Time Cost: " << total_time_cost << " Seconds\n"; // this time cost doesn't not include the time cost of loading bin files
    cout << "Disk Writing Time Cost: " << writingDiskCost << " Seconds\n";
    printf("Averaging over k disk IO time: %f  Computing time: %f\n", writingDiskCost/k, (total_time_cost-writingDiskCost)/k);

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
