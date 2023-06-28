#pragma once
#include "../IO.hpp"
#include <iostream>
#include <cstdio>

using namespace std;

class Config {
public:
    // the config of Index
    int k = 64;              // the number of hash functions
    int interval_limit = 50; // the length threshold of generating compact windows

    // the config of dataset
    string dataset_name = "openwebtext";
    int doc_limit = 8013769; // the amount of docs number

    // the config of bpe encoding way
    int token_num = 50257;

    // the config of zonemap
    int zonemp_interval = 5000; // the stride that decreasing when generating zonemap
    int zoneMpSize = 50257;     // the size of zonemaps under one hashfunction

    // the flag of whether attaching docIndex
    bool if_attachDocIndex = false;

    // the son paths of index
    string cw_dirPath;        // the directory path of  built compact windows
    string index_filePath;    // the file path of built compact windows index file
    string zoneMap_dirPath;   // the directory path of built zomeMap
    string docIndex_filePath; // the file path of build docIndex
    string t_dirPath;         // the directory path of the recorded cw amounts of corresponding text(document)
    string docOfs_dirPath;    // the directory path of offset of each document in the cw_file

    Config(){}

    Config(string _dataset_name, bool _if_attachDocIndex) :
        dataset_name(_dataset_name), if_attachDocIndex(_if_attachDocIndex) {
        if (_dataset_name == "pile")
            config_dataset(_dataset_name, 210607728);
        else if (_dataset_name == "openwebtext")
            config_dataset(_dataset_name, 8013769);
        else if (_dataset_name == "c4")
            config_dataset(_dataset_name, 364868892);
        else
            perror("No given dataset\n");
    }

    Config(int _k, int _interval_limit, string _dataset_name, bool _if_attachDocIndex) :
        k(_k), interval_limit(_interval_limit), dataset_name(_dataset_name), if_attachDocIndex(_if_attachDocIndex) {
        if (_dataset_name == "pile")
            config_dataset(_dataset_name, 210607728);
        else if (_dataset_name == "openwebtext")
            config_dataset(_dataset_name, 8013769);
        else if (_dataset_name == "c4")
            config_dataset(_dataset_name, 364868892);
        else
            perror("No given dataset\n");
    }

    // functions for setting member variables
    void config_dataset(string _dataset_name, int _doc_limit) {
        dataset_name = _dataset_name;
        doc_limit = _doc_limit;
    }

    void config_zonemap(int _zonemp_interval, int _zoneMpSize) {
        zonemp_interval = _zonemp_interval;
        zoneMpSize = _zoneMpSize;
    }

    void set_tokenNums(int _token_num) {
        token_num = _token_num;
    }

    void set_k(int _k) {
        k = _k;
    }

    void set_t(int _t) {
        interval_limit = _t;
    }

    void set_docLimit(int _docLimit) {
        doc_limit = _docLimit;
    }

    // get the directory of those index
    void getFileDirPath(const string &parent_dirPath) {
        string root_dir = getRootDir(parent_dirPath, token_num, k, interval_limit, doc_limit, zoneMpSize, dataset_name);
        cout << "Getting the Directory of " + root_dir << endl;
        getSonDir(root_dir, cw_dirPath, index_filePath, zoneMap_dirPath);

        if (if_attachDocIndex) {
            getDocIndexSonDir(root_dir, docIndex_filePath, t_dirPath, docOfs_dirPath);
        }
    }

    // create the file and directory of those index and so on
    void createFileDirPath(const string &parent_dirPath) {
        string root_dir = createRootDir(parent_dirPath, token_num, k, interval_limit, doc_limit, zoneMpSize, dataset_name);
        cout << "Getting the Directory of " + root_dir << endl;
        createSonDir(root_dir, cw_dirPath, index_filePath, zoneMap_dirPath);

        if (if_attachDocIndex) {
            createDocIndexSonDir(root_dir, docIndex_filePath, t_dirPath, docOfs_dirPath);
        }
    }

    // print out the parameters of the config
    void display_parameters() {
        printf("---------Displaying The Parameters Of Config-----------\n");
        printf("dataset: %s\n", dataset_name.c_str());
        printf("tokenNum: %d ,k: %d , T:%d , zonemap_interval: %d, zoneMpSize: %d\n", token_num, k, interval_limit, zonemp_interval, zoneMpSize);
        printf("All the file location: %s %s %s\n", zoneMap_dirPath.c_str(), index_filePath.c_str(), cw_dirPath.c_str());
    }
};