#pragma once
#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "indexItem.hpp"
using namespace std;


// load index items into indexArr from the given file 
void loadIndexItem(IndexItem ** &indexArr, const int wordNum, const int &k, const string &index_file) {
    printf("------------------Loading Index File------------------\n");
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::in | ios::binary);
    if (!inFile) {
        cout << "error open index file" << endl;
        return;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(IndexItem));
            if (indexArr[i][j].windowsNum < 0) {
                cout << i << " " << j << endl;
                cout << indexArr[i][j].windowsNum << " " << indexArr[i][j].offset << endl;
            }

            assert(indexArr[i][j].windowsNum >= 0);
        }
    }
    inFile.close();
    printf("------------------Index File Loaded------------------\n");
}

// load the vector<int> of a bin file and push back to docs
void loadBin(const string &binFileName, vector<vector<int>> &docs) {
    ifstream ifs(binFileName, ios::binary);
    int size;
    while (ifs.read((char *)&size, sizeof(int))) {
        vector<int> vec(size);
        ifs.read((char *)&vec[0], sizeof(int) * size);
        docs.emplace_back(vec);
    }
    ifs.close();
}

// get all the file names in path and put them in a vector
void getFiles(string path, vector<string> &files) {
    DIR *dr;
    struct dirent *en;
    string file_path;
    dr = opendir(path.c_str()); // open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            // ignore hidden files and folders
            if (en->d_name[0] != '.') {
                const char end = path.back();
                if (end == '/')
                    file_path = path + en->d_name;
                else
                    file_path = path + '/' + en->d_name;
                files.push_back(file_path);
            }
        }
        closedir(dr); // close all directory
    }
}

// load all the bin files in a given dir path and push back all their content to docs
void loadDataDir(const string &dir_path, vector<vector<int>> &docs) {
    vector<string> binFilesNames;
    getFiles(dir_path, binFilesNames);

    for (auto const &fileName : binFilesNames) {
        loadBin(fileName, docs);
    }
}

//检查文件(所有类型,包括目录和文件)是否存在
//返回1:存在 0:不存在
int IsFileExist(const char *path) {
    return !access(path, F_OK);
}

// get the create
string getRootDir(const int &tokenNum, const int &k, const int &T, const int &doc_lim, const int &zoneMpSize, const string &dataset_name) {
    char root_dir_path[50];
    sprintf(root_dir_path, "%s_%dK_%dk_%dT_%dM_%dZP", dataset_name.c_str(), tokenNum / 1000, k, T, doc_lim / 1000000, zoneMpSize);
    if (IsFileExist(root_dir_path)) {
        cout << "get the target root path" << endl;
    } else {
        cout << "Error! Target Root Dir not exist" << endl;
    }

    string str(root_dir_path);
    return str;
}

void getSonDir(const string &root_path, string &cw_dir, string &index_file, string &zonemap_dir) {
    cw_dir = root_path + "/compatWindows/";
    index_file = root_path + "/index.bin";
    zonemap_dir = root_path + "/zonemap/";
}
