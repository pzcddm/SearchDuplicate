#pragma once
#include "cw.hpp"
#include <fstream>
#include <iostream>
using namespace std;
class IndexItem {
public:
    int windowsNum = -1; // how many windows in that list of
    unsigned long long offset = 0;

    IndexItem() {
        windowsNum = -1;
        offset = 0;
    }
    
    bool operator<(const IndexItem &tmp) const {
        if (windowsNum == tmp.windowsNum) {
            return offset < tmp.offset;
        }
        return windowsNum < tmp.windowsNum;
    }

    void display() {
        printf("offset:%llu, windowsNUm: %d\n", offset, windowsNum);
    }

    void getCompatWindows(string cw_files, vector<CW> & res_cws){
        ifstream inFile(cw_files, ios::in | ios::binary); //二进制读方式打开
        if (!inFile) {
            cout << "error open file" << endl;
            return;
        }

        inFile.seekg(offset,ios::beg);//把文件的写指针从文件开头向后移offset个字节

        res_cws.resize(windowsNum);
        inFile.read((char *)&res_cws[0], sizeof(CW)*windowsNum);
        inFile.close();
    }
};