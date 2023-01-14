#include <bits/stdc++.h>
using namespace std;

#include "util/ds/cw.hpp"
// #include "util/indexItem.hpp"
#include "util/ds/bigIndexItem.hpp"
#include "util/IO.hpp"
#include "util/utils.hpp"
#include "util/new_utils.hpp"

#define DEBUG 0

const unsigned BUFFER_SIZE = 1e8;
CW buffer[BUFFER_SIZE];
unsigned long long file_size_debug = 0;

int main(int argc, char **argv) {
    // string scattered_dir = "./index/openwebtext_50K_64k_50T_8M_50257ZP_SCATTERED/";
    // string merged_dir = "./index/openwebtext_50K_64k_50T_8M_50257ZP_Merged/";
    string scattered_dir;
    string merged_dir;

    const string parent_dir = "./index";
    string dataset =  "pile"; // "pile";
    int tokenNum = 50257;
    int doc_limit = 210607728;        // 8013769 210607728
    int K = 32;                       // the number of hash functions
    int INTERVAL_LIMIT = 25;          // set the interval limit for generating compat windows
    const int zonemp_interval = 5000; // the stride that decreasing when generating zonemap
    const int zoneMpSize = 50257;     // the size of zonemaps under one hashfunction
    int scattered_num = 101;

    
    // parse arguments
    for (int i = 0; i < argc; i++) {
        string arg = string(argv[i]);
        if (arg == "-doc_limit") {
            doc_limit = atoi(argv[i + 1]);
        }
        if (arg == "-k") {
            K = atoi(argv[i + 1]);
        }
        if (arg == "-t") {
            INTERVAL_LIMIT = atoi(argv[i + 1]);
        }
    }
    
    // get the path of two directories
    scattered_dir = getScatteredRootDir(parent_dir, tokenNum, K, INTERVAL_LIMIT, doc_limit, zoneMpSize, dataset);
    merged_dir = createRootDir(parent_dir, tokenNum, K, INTERVAL_LIMIT, doc_limit, zoneMpSize, dataset);
   
    int pre_merge_pos = -1; // -1
    auto global_st = LogTime();

    // get the scattered storage son dir
    string scattered_cws_dir, scattered_index_dir, scattered_zonemap_dir;
    getScatteredSonDir(scattered_dir, scattered_cws_dir, scattered_index_dir, scattered_zonemap_dir);

    scattered_num = getDirFileNum(scattered_index_dir);
    scattered_num -= 2;
    printf("scattered_num: %d\n", scattered_num);

    // create the merged son dir
    mkdir(merged_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    string merged_cws_dir, merged_index_file, merged_zonemp_dir;
    createSonDir(merged_dir, merged_cws_dir, merged_index_file, merged_zonemp_dir);

    // load index files into index_vec
    vector<string> index_files(scattered_num);
    for (int i = 0; i < scattered_num; i++) {
        index_files[i] = scattered_index_dir + to_string(i) + ".bin";
    }
    vector<IndexItem **> index_vec(scattered_num);
    for (int i = 0; i < scattered_num; i++) {
        loadIndexItem(index_vec[i], tokenNum, K, index_files[i]);
    }

    // Initialize merged_index
    BigIndexItem **merged_indexArr;
    merged_indexArr = new BigIndexItem *[K];
    for (int i = 0; i < K; i++) {
        merged_indexArr[i] = new BigIndexItem[tokenNum];
    }

    // build previous index (if there is a sudden error or stop in the last time building, then this part should be used)
    for (int i = 0; i <= pre_merge_pos; i++) {
        unsigned long long global_offset = 0;
        unsigned long long cur_token_offset;
        for (int j = 0; j < tokenNum; j++) {
            long long cur_window_num = 0;

            // thet starting offset of current token in the merged_cws_file
            cur_token_offset = global_offset;
            for (int k = 0; k < scattered_num; k++) {
                int tmp_windowsNum = index_vec[k][i][j].windowsNum;
                assert(tmp_windowsNum >= 0 && tmp_windowsNum <= INT_MAX);
                // update variables
                global_offset += sizeof(CW) * tmp_windowsNum;
                cur_window_num += (long long)tmp_windowsNum;
                assert(cur_window_num >= 0);
            }
            assert(cur_window_num >= 0);
            merged_indexArr[i][j].windowsNum = cur_window_num;
            assert(merged_indexArr[i][j].windowsNum >= 0);
            merged_indexArr[i][j].offset = cur_token_offset;
        }
    }

    // Start iterating each hash function's scattered data
    printf("----------- starting merging--------------\n");

    for (int i = pre_merge_pos + 1; i < K; i++) {
        auto local_st = LogTime();

        // Intialize ofstream
        string merged_cws_file = merged_cws_dir + to_string(i) + ".bin";
        ofstream merged_cws_ofstream(merged_cws_file, ios::out | ios::binary);
        string merged_zonemap_file = merged_zonemp_dir + to_string(i) + ".bin";
        ofstream merged_zonemap_ofstream(merged_zonemap_file, ios::out | ios::binary);

        if ((!merged_cws_ofstream) || (!merged_zonemap_ofstream)) {
            cout << "error open file" << endl;
        }

        // Initialize ifstream
        vector<ifstream> cws_ifstreams(scattered_num);
        vector<ifstream> zmp_ifstreams(scattered_num);

        if (DEBUG) {
            file_size_debug = 0;
        }

        for (int j = 0; j < scattered_num; j++) {
            string tmp_cw_file = scattered_cws_dir + to_string(i) + "/" + to_string(j) + ".bin";
            cws_ifstreams[j].open(tmp_cw_file, ios::in | ios::binary);

            string tmp_zmp_file = scattered_zonemap_dir + to_string(i) + "/" + to_string(j) + ".bin";
            zmp_ifstreams[j].open(tmp_zmp_file, ios::in | ios::binary);

            if (DEBUG) {
                const auto begin = cws_ifstreams[j].tellg();
                cws_ifstreams[j].seekg(0, ios::end);
                const auto end = cws_ifstreams[j].tellg();
                file_size_debug += (end - begin);
                cws_ifstreams[j].seekg(0, ios::beg);
            }
        }

        unsigned long long global_offset = 0;
        unsigned long long cur_token_offset;

        vector<pair<int, unsigned long long>> zonemap_buffer;
        // Pratically Merging
        for (int j = 0; j < tokenNum; j++) {
            // thet starting offset of current token in the merged_cws_file
            cur_token_offset = global_offset;
            long long cur_window_num = 0;
            for (int k = 0; k < scattered_num; k++) {
                unsigned tmp_windowsNum = index_vec[k][i][j].windowsNum;
                unsigned long long ori_offset = index_vec[k][i][j].offset;

                assert(tmp_windowsNum >= 0);
                if (tmp_windowsNum > BUFFER_SIZE) {
                    cout << tmp_windowsNum << endl;
                }
                // assert(tmp_windowsNum <= BUFFER_SIZE);
                // read cws and write them
                unsigned left_windowsNum = tmp_windowsNum;
                while (left_windowsNum > BUFFER_SIZE) {
                    cws_ifstreams[k].read((char *)buffer, sizeof(CW) * BUFFER_SIZE);
                    merged_cws_ofstream.write((char *)buffer, sizeof(CW) * BUFFER_SIZE);
                    left_windowsNum -= BUFFER_SIZE;
                }
                cws_ifstreams[k].read((char *)buffer, sizeof(CW) * left_windowsNum);
                merged_cws_ofstream.write((char *)buffer, sizeof(CW) * left_windowsNum);

                // read Zonemap and save it into Zonemap_buffer
                unsigned tid;
                unsigned zp_len;
                zmp_ifstreams[k].read((char *)&tid, sizeof(unsigned));    // read token_id
                zmp_ifstreams[k].read((char *)&zp_len, sizeof(unsigned)); // read zonemap_length
                assert(tid == j);
                for (int l = 0; l < zp_len; l++) {
                    int text_id;
                    unsigned long long offset;
                    zmp_ifstreams[k].read((char *)&text_id, sizeof(int));
                    zmp_ifstreams[k].read((char *)&offset, sizeof(unsigned long long));
                    offset += global_offset - ori_offset; // calculate the new offset in the merged file
                    zonemap_buffer.emplace_back(text_id, offset);
                }

                // update variables
                global_offset += sizeof(CW) * tmp_windowsNum;
                cur_window_num += (long long)tmp_windowsNum;
                assert(cur_window_num >= 0);
            }

            // Write zonemap files
            unsigned cur_token = j;
            unsigned cur_zp_len = zonemap_buffer.size();
            merged_zonemap_ofstream.write((char *)&cur_token, sizeof(unsigned));
            merged_zonemap_ofstream.write((char *)&cur_zp_len, sizeof(unsigned));
            for (auto const &pir : zonemap_buffer) {
                merged_zonemap_ofstream.write((char *)&pir.first, sizeof(int));                 // write text_id
                merged_zonemap_ofstream.write((char *)&pir.second, sizeof(unsigned long long)); // write offset
            }
            zonemap_buffer.clear();
            // Record indexItem
            assert(cur_window_num >= 0);
            merged_indexArr[i][j].windowsNum = cur_window_num;
            merged_indexArr[i][j].offset = cur_token_offset;
            assert(cur_token_offset <= global_offset);
        }

        if (DEBUG) {
            cout << "global_offset " << global_offset << "file_size_debug" << file_size_debug << endl;
            assert(global_offset == file_size_debug);
        }
        // Close ofstream and ifstream
        for (int j = 0; j < scattered_num; j++) {
            cws_ifstreams[j].close();
            zmp_ifstreams[j].close();
        }
        merged_cws_ofstream.close();
        merged_zonemap_ofstream.close();

        printf("Local Merging of current hash function cost : %f\n", RepTime(local_st));
    }

    printf("------------------Writing Index File------------------\n");
    ofstream index_ofstream(merged_index_file, ios::out | ios::binary);
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < tokenNum; j++) {
            if (merged_indexArr[i][j].windowsNum < 0) {
                cout << i << " " << j << endl;
            }
            assert(merged_indexArr[i][j].windowsNum >= 0);
            index_ofstream.write((char *)&merged_indexArr[i][j], sizeof(BigIndexItem));
        }
    }
    index_ofstream.close();
    printf("------------------Index File Writed------------------\n");

    printf("Total cost : %f\n", RepTime(global_st));
}