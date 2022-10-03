#include <cstdio>
#include <cctype>
#include <iostream>
#include <fstream>
#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/cw.hpp"
#include "util/indexItem.hpp"
using namespace std;

int main() {
    // Load the map of word to int
    unordered_map<string, int> word2id;
    const string wiki_words_file_name = "wiki_test_words.txt";
    loadWord2id(wiki_words_file_name, word2id);
    int wordNum = word2id.size(); // the number of tokens
    cout << wordNum << endl;
    int k = 10; // the number of hash functions

    IndexItem **indexArr;
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    // open the sorted compat windows file
    string cw_file = "CompatWindows";
    ifstream inFile(cw_file, ios::in | ios::binary); //二进制读方式打开
    if (!inFile) {
        cout << "error" << endl;
        return 0;
    }

    Wrapped_CW tmp_wrappedCW;
    unsigned long long offset = 0;

    printf("wrappedCW的字节数量为%lu\n", sizeof(tmp_wrappedCW));
    while (inFile.read((char *)&tmp_wrappedCW, sizeof(tmp_wrappedCW))) { //一直读到文件结束
        int token_id = tmp_wrappedCW.token_id;
        int ith_hash = tmp_wrappedCW.ith_hash;

        assert(token_id < wordNum);
        assert(ith_hash < k);
        // 还没碰到过这个IndexItem时
        if (indexArr[ith_hash][token_id].windowsNum == -1) {
            indexArr[ith_hash][token_id].windowsNum = 1;
            indexArr[ith_hash][token_id].offset = offset;
        } else {
            indexArr[ith_hash][token_id].windowsNum++;
        }

        // increase offset
        offset += sizeof(tmp_wrappedCW);
    }

    cout << "offset: " << offset << endl;

    // //display some IndexItem for debug
    // for(int i = 0;i<wordNum;i++){
    //     if(indexArr[0][i].windowsNum != -1){
    //         cout<< "token_id: "<< i<<endl;
    //         for(int j=0;j<k;j++){
    //             indexArr[j][i].display();
    //         }
    //     }
    // }

    // Write the whole index into index file
    string index_file = "indexFile";

    ofstream outFile(index_file, ios::out | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            outFile.write((char *)&indexArr[i][j], sizeof(IndexItem));
        }
    }
    outFile.close();
    
}