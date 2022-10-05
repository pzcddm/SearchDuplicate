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
    ifstream inFile("CompatWindow", ios::in | ios::binary); //二进制读方式打开
    int token_id = 803;
    //  inFile.seekg(0,ios::beg);//把文件的写指针从文件开头向后移offset个字节
        Wrapped_CW tmp_wrappedCW;
        unsigned long long  offset =0;
        for(int i =0 ;i<10;i++){
            inFile.read((char *)&tmp_wrappedCW, sizeof(tmp_wrappedCW));
            offset += sizeof(tmp_wrappedCW);
            //check if token_id matches the id user wants
            // if(token_id != tmp_wrappedCW.token_id)
            // {   
            //     cout<<i<<endl;
            //     //printf("offset: %lu token_id: %d try tokenid: %d\n",offset, token_id,tmp_wrappedCW.token_id);
            //     tmp_wrappedCW.display();
            // }
            // assert(token_id == tmp_wrappedCW.token_id);
            // res_cws.push_back(tmp_wrappedCW.cw);
        }
        cout << offset <<endl;
        cout<<(unsigned long) (inFile.tellg())<<endl;
        inFile.close();
        return 0;
}   
