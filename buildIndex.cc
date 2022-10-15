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
    const string cw_dir = "compatWindows/openwebtext/";
    const string index_file = "index/indexOpenWebText.bin";
    const int k = 100; // the number of hash functions
    const int tokenNum = 64000;

    IndexItem **indexArr;
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[tokenNum];
    }

    
    Wrapped_CW tmp_wrappedCW;

    printf("wrappedCW的字节数量为%lu\n", sizeof(tmp_wrappedCW));
    unsigned long long compat_windows_num = 0;

    // indicate which hash function
    for (int ith_hash = 0; ith_hash < k; ith_hash++) {
        string cw_file = cw_dir + to_string(ith_hash) + ".bin";

        ifstream inFile(cw_file, ios::in | ios::binary);
        // open the sorted compat windows file

        if (!inFile) {
            cout << cw_file<< " open error" << endl;
            return 0;
        }
        unsigned long long offset = 0;                    //二进制读方式打开
        while (inFile.read((char *)&tmp_wrappedCW, sizeof(tmp_wrappedCW))) { //一直读到文件结束

            int token_id = tmp_wrappedCW.token_id;

            assert(token_id >= 0 && token_id < tokenNum);
            // 还没碰到过这个IndexItem时
            if (indexArr[ith_hash][token_id].windowsNum == -1) {
                indexArr[ith_hash][token_id].windowsNum = 1;
                indexArr[ith_hash][token_id].offset = offset;
            } else {
                indexArr[ith_hash][token_id].windowsNum++;
            }

            // increase offset
            offset = offset + sizeof(tmp_wrappedCW);
            compat_windows_num++;
        }
        cout << cw_file <<" loaded "<<" total offset: " << offset << endl;

    }

    cout << "compat windows num" << compat_windows_num << endl;

    // Write the whole index into index file
    printf("------------------Writing Index File------------------\n");

    ofstream outFile(index_file, ios::out | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < tokenNum; j++) {
            outFile.write((char *)&indexArr[i][j], sizeof(IndexItem));
        }
    }
    outFile.close();
    printf("------------------Index File Writed------------------\n");

    return 0;
}