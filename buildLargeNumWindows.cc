#include <bits/stdc++.h>
#include <fstream>
#include <omp.h>
// #include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/IO.hpp"
#include "util/ds/cw.hpp"
#include "util/ds/indexItem.hpp"

int INTERVAL_LIMIT;
int tokenNum;

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

    assert(doc[ret.second] >= 0 && doc[ret.second] < tokenNum);
    res_cws[doc[ret.second]].emplace_back(doc_id, l, ret.second, r);
    partition(doc_id, doc, seg, l, ret.second - 1, res_cws);
    partition(doc_id, doc, seg, ret.second + 1, r, res_cws);
}

// get the compat windows of one document
void generateCompatWindow(const int &doc_id, const vector<int> &doc, vector<pair<int, int>> &hf, int ith_hf, vector<vector<CW>> &res_cws, vector<pair<int, int>> &seg) {
    assert(doc_id < 210607728);
    assert(INTERVAL_LIMIT >= 1);
    if(doc.size()<INTERVAL_LIMIT)
        return;
    assert(doc.size()>=1);
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
void display_parameters(const int &tokenNum, const int &k, const int &T, const int &zonemap_interval, const int &zoneMpSize) {
    printf("tokenNum: %d ,k: %d , T:%d , zonemap_interval: %d, zoneMpSize: %d\n", tokenNum, k, T, zonemap_interval, zoneMpSize);
}

string createRootDir(const int &tokenNum, const int &k, const int &T, const int &doc_lim, const int &zoneMpSize, const string &dataset_name) {
    char root_dir_path[50];
    sprintf(root_dir_path, "%s_%dK_%dk_%dT_%dM_%dZP_SCATTERED", dataset_name.c_str(), tokenNum / 1000, k, T, doc_lim / 1000000, zoneMpSize);
    mkdir(root_dir_path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);

    string str(root_dir_path);
    return str;
}

// the directory for building large amount of compact windows is below:
// /root_path/
//  -/compatWindows/
//  --/0...1...2/
//  ---0.bin...1.bin...
//  -/zonemap/
//  --/0...1...2/
//  ---0.bin...1.bin...
//  -/index/
//  --index_0.bin...index_1.bin
void createSonDir(const string &root_path, string &cw_dir, string &index_dir, string &zonemap_dir, int k) {
    cw_dir = root_path + "/compatWindows/";
    index_dir = root_path + "/index/";
    zonemap_dir = root_path + "/zonemap/";

    // create directory for cw dir and zonemap_dir
    mkdir(cw_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    mkdir(zonemap_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    mkdir(index_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    
    // create k dirs in zonemap_dir and cw_dir
    for(int i =0;i<k;i++){
        string tmp_dir = cw_dir+to_string(i)+"/";
        mkdir(tmp_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);

        tmp_dir = zonemap_dir+to_string(i)+"/";
        mkdir(tmp_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    }

    printf("Directory Made\n");
}

// Todo: Build Index to memory
int main(int argc, char **argv) {
    // string scr_dir = "../openwebtext_64K_vocal/";
    string src_file = "../dataset_tokenizedGbt2/pile_gpt2.bin";
    // string dataset_name = "pile";
    // string src_file = "../dataset_tokenizedGbt2/openwebtext_gpt2.bin";
    string dataset_name = "pile";
    tokenNum = 50257;
    int doc_limit = 210607728;          // 8013769 210607728
    int epoch_docNum = doc_limit / 100; // the maximun number of documents that are iterated in a epoch
    int k = 32;                         // the number of hash functions
    INTERVAL_LIMIT = 50;                // set the interval limit for generating compat windows
    const int zonemp_interval = 5000;   // the stride that decreasing when generating zonemap
    const int zoneMpSize = 50257;        // the size of zonemaps under one hashfunction

    for (int i = 0; i < argc; i++) {
        string arg = string(argv[i]);
        if (arg == "-doc_limit") {
            doc_limit = atoi(argv[i + 1]);
        }
        if (arg == "-k") {
            k = atoi(argv[i + 1]);
        }
        if (arg == "-t") {
            INTERVAL_LIMIT = atoi(argv[i + 1]);
        }
    }

    // Storage location of results and create them
    string cw_dir;
    string index_dir;
    string zoneMap_dir;
    string root_dir = createRootDir(tokenNum, k, INTERVAL_LIMIT, doc_limit, zoneMpSize, dataset_name);
    createSonDir(root_dir, cw_dir, index_dir, zoneMap_dir, k);

    // the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
    vector<pair<int, int>> hf;
    for (int i = 1; i <= k; i++) generateHashFunc(i, hf);
    
    double readFileTime = 0;
    auto global_st = LogTime();
    // time cost of writing files
    double writingDiskCost = 0;
    unsigned long long total_cws_amount = 0;

    ifstream ifs(src_file, ios::binary);
    // iterate
    int doc_cnt = 0;
    int epochs = 0;
    vector<vector<int>> docs;
    while (doc_cnt < doc_limit) {

        auto readOneDocumentSt =  LogTime();
        // read one document
        int size;
        
        ifs.read((char *)&size, sizeof(int));
        if(size<=1){
            cout<<"Low size: "<<size<<endl;
        }

        vector<int> vec(size);
        ifs.read((char *)&vec[0], sizeof(int) * size);
        docs.emplace_back(vec);
        
        readFileTime += RepTime(readOneDocumentSt);
        doc_cnt++;
        // if(doc_cnt == 733070){
        //     for(auto const & tmp:vec){
        //         cout<<tmp<<endl;
        //     }
        // }
        // Generate cws and write them from partial documents
        if(doc_cnt % epoch_docNum == 0 || doc_cnt == doc_limit){
            printf("------------------Partial Documents Loaded------------------\n");
            printf("Epoch: %d Current Doc_Cnt :%d \n", epochs, doc_cnt);

            

            
            // Index Item
            IndexItem **indexArr;
            indexArr = new IndexItem *[k];
            for (int i = 0; i < k; i++) {
                indexArr[i] = new IndexItem[tokenNum];
            }

            printf("------------------Generating Compat Windows------------------\n");
            // generate compat windows for every document for every hash function
            
            int thread_num = omp_get_max_threads();
            vector<vector<pair<int, int>>> segtrees(thread_num, vector<pair<int, int>>(MAX_LENGTH));

            for (int i = 0; i < k; i++) {
                vector<vector<vector<CW>>> tmp_vetor(thread_num, vector<vector<CW>>(tokenNum)); // Three-dimensional(threads, tokens, compatwindows) arrays
                                                                                                // Genrate Compat windows under the current hash function

                int pre_doc_id = doc_cnt-docs.size();
                printf("%d\n", docs.size());

#pragma omp parallel for
                for (int doc_id = pre_doc_id; doc_id < doc_cnt; doc_id++) {
                    // cout<<doc_id<<endl;
                    int thread_id = omp_get_thread_num();
                    assert(doc_id-pre_doc_id<docs.size());
                    generateCompatWindow(doc_id, docs[doc_id-pre_doc_id], hf, i, tmp_vetor[thread_id], segtrees[thread_id]);
                }

                cout << "Partition Algo Over" << endl;                
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

                vector<unsigned> cws_len(tokenNum);
                for (int j = 0; j < tokenNum; j++) {
                    cws_len[j] = res_cws[j].size();
                }
                vector<vector<pair<int, unsigned long long>>> zonemap(zoneMpSize);
                int zonemp_cnt = 0;

                // Timer ON
                auto writingDiskTimer = LogTime();

                // write these cws into a file
                string save_path = cw_dir + to_string(i)+"/"+to_string(epochs) + ".bin";
                ofstream outFile(save_path, ios::out | ios::binary);

                unsigned long long offset = 0;
                for (int j = 0; j < tokenNum; j++) {
                    // create zoneMap[zonemp_cnt]
                    unsigned long long tmp_offset = offset;
                    int stride = 0;
                    int pre_text_id = -1;
                    for (auto const &cw : res_cws[j]) {

                        // write this cw
                        outFile.write((char *)&cw, sizeof(cw));

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

                    indexArr[i][j].windowsNum = res_cws[j].size();
                    indexArr[i][j].offset = offset;
                    offset = offset + sizeof(CW) * indexArr[i][j].windowsNum;
                }
                outFile.close();
                // writing zonemap
                cout << "Zone map writing" << endl;
                string zonemap_path = zoneMap_dir + to_string(i) + "/"+to_string(epochs) + ".bin";
                ofstream zpFile(zonemap_path, ios::out | ios::binary);

                for (int j = 0; j < tokenNum; j++) {
                    unsigned tid = j;
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
                cout << "Current total compat window amount: " << total_cws_amount << endl;
            }

            printf("------------------Writing Index File------------------\n");
            string index_file = index_dir + to_string(epochs) +".bin";
            ofstream outFile(index_file, ios::out | ios::binary);
            for (int i = 0; i < k; i++) {
                for (int j = 0; j < tokenNum; j++) {
                    outFile.write((char *)&indexArr[i][j], sizeof(IndexItem));
                }
            }
            outFile.close();
            printf("------------------Index File Writed------------------\n");

            
            epochs++;

            // system("keep-job 48");

            // clear docs 
            docs.clear();
        }
    }
    
    ifs.close();

    cout << "File Reading Cost: "<< readFileTime<<"s "<<endl;
    cout << "total cost time :"<< RepTime(global_st) <<"s "<<endl;
    cout << "doc number: " << doc_cnt << endl;

    printf("------------------Compat Windows Generated------------------\n");
    cout << "sort complete and write cws into file" << endl;

    // Timer Off
    double total_time_cost = RepTime(global_st);
    display_parameters(tokenNum, k, INTERVAL_LIMIT, zonemp_interval, zoneMpSize);
    cout << "Compat Windows Generation, Sorting, and Saving Time Cost: " << total_time_cost << " Seconds\n"; // this time cost doesn't not include the time cost of loading bin files
    cout << "Disk Writing Time Cost: " << writingDiskCost << " Seconds\n";
    printf("Averaging over k disk IO time: %f  Computing time: %f compact windows amount: %f\n", writingDiskCost / k, (total_time_cost - writingDiskCost) / k, total_cws_amount * 1.0 / k);

    
}
