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
    
    void display() {
        printf("offset:%llu, windowsNUm: %d\n", offset, windowsNum);
    }

    void getCompatWindows(string cw_files, vector<CW> & res_cws, int token_id){
        ifstream inFile(cw_files, ios::in | ios::binary); //二进制读方式打开
        if (!inFile) {
            cout << "error open file" << endl;
            return;
        }

        inFile.seekg(offset,ios::beg);//把文件的写指针从文件开头向后移offset个字节
        Wrapped_CW tmp_wrappedCW;

        res_cws.resize(windowsNum);
        for(int i =0 ;i<windowsNum;i++){
            inFile.read((char *)&tmp_wrappedCW, sizeof(tmp_wrappedCW));

            //check if token_id matches the id user wants
            if(token_id != tmp_wrappedCW.token_id)
            {   
                display();
                printf("offset: %llu token_id: %d try tokenid: %d\n",offset, token_id,tmp_wrappedCW.token_id);
                tmp_wrappedCW.display();
            }
            assert(token_id == tmp_wrappedCW.token_id);
            res_cws[i]=tmp_wrappedCW.cw;
        }
        inFile.close();
    }
};