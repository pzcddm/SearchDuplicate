#pragma once
#include "cw.hpp"
#include <fstream>
#include <iostream>
using namespace std;
class IndexItem {
public:
    unsigned windowsNum = -1; // how many windows in that list of
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
        if(windowsNum<0){
            cout<<windowsNum<<endl;
        }
        assert(windowsNum>=0);
        res_cws.resize(windowsNum);
        inFile.read((char *)&res_cws[0], sizeof(CW)*windowsNum);
        inFile.close();
    }

    void getOneDocumentCWs(string cw_files,const unsigned long long & docOfs, vector<CW> & res_cws){
        ifstream inFile(cw_files, ios::in | ios::binary); //二进制读方式打开
        if (!inFile) {
            cout << "error open file" << endl;
            return;
        }

        inFile.seekg(docOfs,ios::beg);//把文件的写指针从文件开头向后移offset个字节

        CW tmp_cw;
        inFile.read((char *)&tmp_cw, sizeof(CW));
        unsigned target_docId = tmp_cw.T;

        while((!(inFile && inFile.peek() == EOF)) && target_docId == tmp_cw.T){
            res_cws.emplace_back(tmp_cw);
            inFile.read((char *)&tmp_cw, sizeof(CW));
        }
        
        inFile.close();
    }

    // the CWS from these multiple document will be stored in the res_vet sequentially
    void getMutipleDocumentCWs(const string& cw_file,const vector<pair<unsigned long long, int>> &ofs_vet, vector<CW> & res_cws){
        printf("the size of ofs_vet: %lu\n", ofs_vet.size());
        // count the total amount of cws in this load
        unsigned long long total_cws_amount = 0;
        for(auto const & pa : ofs_vet){
            total_cws_amount += pa.second;
        }

        res_cws.resize(total_cws_amount);
        
        ifstream inFile(cw_file, ios::in | ios::binary); //二进制读方式打开
        if (!inFile) {
            cout << "error open file" << cw_file << endl;
            return;
        }

        unsigned long long count = 0;
        for(auto const & pa : ofs_vet){
            inFile.seekg(pa.first,ios::beg);
            inFile.read((char *)&res_cws[count], sizeof(CW)* pa.second);
            count += pa.second;
        }

        inFile.close();
    }   
};
