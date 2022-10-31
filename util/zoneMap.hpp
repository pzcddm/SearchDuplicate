#pragma once
#include <bits/stdc++.h>
#include "cw.hpp"

using namespace std;

class ZoneMaps{
    // the map that maps token id to zonemap_id
    vector<unordered_map<unsigned, int>> tokenId2index;

    // the 
    vector<vector<unsigned>> longTokenIds;
    vector<vector<vector<pair<int, unsigned long long>>>> zoneMaps;

    int map_size;
    public:

        ZoneMaps(){}

        ZoneMaps(int _max_k, int _map_size){
            zoneMaps.resize(_max_k);
            tokenId2index.resize(_max_k);
            map_size = _map_size;
        }

        void load(const string & zonemap_dir){
            for (int i = 0; i < zoneMaps.size(); i++) {
                // open file
                string zonemapFile = zonemap_dir + to_string(i) + ".bin";
                ifstream inFile(zonemapFile, ios::in | ios::binary);

                zoneMaps[i].resize(map_size);

                for (int j = 0; j < map_size; j++) {
                    unsigned tid;
                    unsigned zp_len;

                    inFile.read((char *)&tid, sizeof(unsigned));    // read token_id
                    inFile.read((char *)&zp_len, sizeof(unsigned)); // read zonemap_length
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

        bool if_in_zonemap(const int& ith_khash, const int & token_id){
            return tokenId2index[ith_khash].count(token_id);
        }
        // Given speicalized hash function id, token_id, and text_id. load the corresponding compact windows into text_cws
        void getCWinText(ifstream& inFile, const int & ith_khash, const int & token_id, const int & text_id, vector<CW>& text_cws){
            if(tokenId2index[ith_khash].count(token_id) == 0){
                printf("ith_khash :%d token_id:%d not in this zonemap:\n",ith_khash,token_id);
                return;
            }
                
            assert(tokenId2index[ith_khash].count(token_id));
            const auto &zonemp = zoneMaps[ith_khash][tokenId2index[ith_khash][token_id]];
        
            // find the first pair that larger than (candid_text,0ULL)
            auto it = upper_bound(zonemp.begin(), zonemp.end(), make_pair(text_id, 0ULL));
            if (it == zonemp.begin()) {
                return;
            }

            it--;
            pair<int, unsigned long long> val = *it;
            assert(val.first <= text_id);

            unsigned long long offset = val.second;
            inFile.seekg(offset, ios::beg);
            CW tmp_cw;

            // load compat windows under specified T
            while (inFile.read((char *)&tmp_cw, sizeof(CW))) {
                if (tmp_cw.T > text_id) { // because the cw is ordered
                    break;
                }

                if (tmp_cw.T == text_id) {
                    text_cws.emplace_back(tmp_cw);
                }
            }
            return;
        }
};