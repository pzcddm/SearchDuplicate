#pragma once
/*
    The purpose is to save the space when loading text informations of all cws
    So in the search process, the program will first load doc index and then check which document meets the requirement
    Then it will load the cws to the text of corresponding texts

    Therefore, this docIndex needs to use two file:
    1. A file that saves t_offset
    2. A file that saves ofs_offset
*/
#include <fstream>
#include <iostream>
using namespace std;
class DocIndex {
public:
    unsigned docs_num = -1;            // how many documents does this token appear
    unsigned long long t_offset = 0;   // the offset of this token docs list in the file that saves all the tokens' doc_id
    unsigned long long ofs_offset = 0; // the offset of this token offset list in the file that saves all the offsets of tokens' cws list offset

    DocIndex() {
    }

    DocIndex(const unsigned &_docs_num, const unsigned long long &_t_offset, const unsigned long long &_ofs_offset) :
        docs_num(_docs_num), t_offset(_t_offset), ofs_offset(_ofs_offset) {
    }

    bool operator<(const DocIndex &tmp) const {
        if (docs_num == tmp.docs_num) {
            return t_offset < t_offset;
        }
        return docs_num < tmp.docs_num;
    }

    void display() {
        printf("docs_num: %u, t_offset:%llu, ofs_offset: %llu\n", docs_num, t_offset, ofs_offset);
    }

    void getDocsList(const string &t_file, vector<int> &res_t_list) {
        ifstream inFile(t_file, ios::in | ios::binary); // Open in binary read mode

        // Check whether the file is opened successfully
        if (!inFile) {
            cout << "error open t_file file" << endl;
            return;
        }

        inFile.seekg(t_offset, ios::beg); // Move the read pointer of the file
        if (docs_num == -1) {
            printf("Error!!!, the  docs_num of this docIndex is the default value (which is a impossible num)\n");
        }

        // Read the docs_id and assign them into t_list
        res_t_list.resize(docs_num);
        inFile.read((char *)&res_t_list[0], sizeof(int) * docs_num);
        inFile.close();
    }

    unsigned long long getSpecifiedDocOffset(const string &ofs_file, const unsigned &pos) {
        ifstream inFile(ofs_file, ios::in | ios::binary); // Open in binary read mode

        // Check whether the file is opened successfully
        if (!inFile) {
            cout << "error open ofs_file file" << endl;
            return 0;
        }

        auto corr_ofs = ofs_offset + pos * sizeof(unsigned long long);

        inFile.seekg(corr_ofs, ios::beg); // Move the read pointer of the file

        unsigned long long res_offset;
        inFile.read((char *)&res_offset, sizeof(unsigned long long));
        inFile.close();

        return res_offset;
    }
};
