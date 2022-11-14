#include <bits/stdc++.h>
using namespace std;

#include "util/cw.hpp"
#include "util/indexItem.hpp"
#include "util/IO.hpp"
#include "util/util.hpp"

int main(){
    string scattered_dir = "./openwebtext_50K_64k_50T_8M_50257ZP_SCATTERED/";
    string merged_dir = "openwebtext_50K_64k_50T_8M_50257ZP";
    tokenNum = 50257;
    int doc_limit = 8013769;          // 8013769 210607728
    int k = 64;                         // the number of hash functions
    INTERVAL_LIMIT = 50;                // set the interval limit for generating compat windows
    const int zonemp_interval = 5000;   // the stride that decreasing when generating zonemap
    const int zoneMpSize = 50257;        // the size of zonemaps under one hashfunction
    const int scattered_num = 101;

    auto global_st = LogTime();
    
    // get the scattered storage son dir
    string scattered_cws_dir, scattered_index_dir, scattered_zonemap_dir;
    getScatteredSonDir(scattered_dir, scattered_cws_dir, scattered_index_dir, scattered_zonemap_dir);

    // create the merged son dir
    string merged_cws_dir, merged_index_file, merged_zonemp_dir;
    createSonDir(merged_dir, merged_cws_dir, merged_index_file, merged_zonemp_dir);

    // load index files into index_vec
    vector<string> index_files(scattered_num);
    for(int i = 0 ; i< scattered_num;i++){
        index_files[i] = scattered_index_dir+to_string(i)+".bin";
    }
    vector<IndexItem**> index_vec(scattered_num);
    for(int i = 0 ; i< scattered_num;i++){
        loadIndexItem(index_vec[i], tokenNum , k, index_files[i])
    }
    
    // Initialize merged_index
    IndexItem ** merged_indexArr;
    merged_indexArr = new IndexItem *[k];
    for (int i = 0; i < k; i++) {
        merged_indexArr[i] = new IndexItem[tokenNum];
    }

    // Start Iterate each hash function's scattered data
    printf("----------- starting merging--------------\n")
    for(int i = 0 ;i<k;i++){
        auto local_st = LogTime();

        // Intialize ofstream
        string merged_cws_file = merged_cws_dir+to_string(i)+".bin";
        ofstream merged_cws_ofstream(merged_cws_file, ios::out | ios::binary);
        string merged_zonemap_file = merged_zonemp_dir+to_string(i)+".bin";
        ofstream merged_zonemap_ofstream(merged_zonemap_file, ios::out | ios::binary);

        // Initialize ifstream
        vector<ifstream> cws_ifstreams(scattered_num);
        vector<ifstream> zmp_ifstreams(scattered_num);

        for(int j =0;j<scattered_num;j++){
            string tmp_cw_file = scattered_cws_dir+to_string(i)+"/"+to_string(j)+".bin";
            cws_ifstreams[j].open(tmp_cw_file, ios::in | ios::binary);

            string tmp_zmp_file = scattered_zonemap_dir+to_string(i)+"/"+to_string(j)+".bin";
            zmp_ifstreams[j].open(tmp_zmp_file, ios::in | ios::binary);
        }

        // Close ofstream and ifstream
        for(int j =0;j<scattered_num;j++){
            cws_ifstreams[j].close();
            zmp_ifstreams[j].close();
        }
        merged_cws_ofstream.close();
        merged_zonemap_ofstream.close()
    }
    printf("------------------Writing Index File------------------\n");

    ofstream outFile(index_file, ios::out | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < tokenNum; j++) {
            outFile.write((char *)&indexArr[i][j], sizeof(IndexItem));
        }
    }
    outFile.close();
    printf("------------------Index File Writed------------------\n");

    printf("Total cost : %f\n", RepTime(global_st));
}   