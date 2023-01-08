#pragma once
#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ds/indexItem.hpp"
#include "ds/docIndex.hpp"
#include "ds/bigIndexItem.hpp"
using namespace std;

// load docIndexs into docIndexArr from the given file 
void loadDocIndex(DocIndex ** &docIndexArr, const int wordNum, const int &k, const string &docIndex_file_path) {
    printf("------------------Loading DocIndex File------------------\n");
    docIndexArr = new DocIndex *[k];

    for (int i = 0; i < k; i++) {
        docIndexArr[i] = new DocIndex[wordNum];
    }

    ifstream inFile(docIndex_file_path, ios::in | ios::binary);
    if (!inFile) {
        cout << "error open DocIndex file" << endl;
        return;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&docIndexArr[i][j], sizeof(DocIndex));
            if (docIndexArr[i][j].docs_num == -1) {
                cout << i << " " << j << endl;
                cout << docIndexArr[i][j].t_offset << " " << docIndexArr[i][j].ofs_offset << endl;
            }
        }
    }
    inFile.close();
    printf("------------------Index DocIndex Loaded------------------\n");
}

// load index items into indexArr from the given file 
void loadBigIndexItem(BigIndexItem ** &indexArr, const int wordNum, const int &k, const string &index_file) {
    // printf("------------------Loading Index File------------------\n");
    indexArr = new BigIndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new BigIndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::in | ios::binary);
    if (!inFile) {
        cout << "error open index file" << endl;
        return;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(BigIndexItem));
            if (indexArr[i][j].windowsNum < 0) {
                cout << i << " " << j << endl;
                cout << indexArr[i][j].windowsNum << " " << indexArr[i][j].offset << endl;
            }

            assert(indexArr[i][j].windowsNum >= 0);
        }
    }
    inFile.close();
    // printf("------------------Index File Loaded------------------\n");
}

// load index items into indexArr from the given file 
void loadIndexItem(IndexItem ** &indexArr, const int wordNum, const int &k, const string &index_file) {
    // printf("------------------Loading Index File------------------\n");
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::in | ios::binary);
    if (!inFile) {
        cout << "error open "<<index_file<<" index file" << endl;
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
    // printf("------------------Index File Loaded------------------\n");
}

void writeBin(const vector<vector<int>> & vecs, const string & binFileName){
    ofstream ofs(binFileName, ios::binary);
    for(auto const & vec:vecs){
        int size = vec.size();
        ofs.write((char *)&size, sizeof(int));
        for(auto const & tmp: vec){
            ofs.write((char *)&tmp, sizeof(int));
        }
    }
    ofs.close();
}

// load the vector<int> of a bin file and push back to docs
void loadBin(const string &binFileName, vector<vector<int>> &docs) {
    ifstream ifs(binFileName, ios::binary);
    if (!ifs) {
        cout << "error open bin file" << endl;
        return;
    }
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
string getRootDir(const string & parent_dir, const int &tokenNum, const int &k, const int &T, const int &doc_lim, const int &zoneMpSize, const string &dataset_name) {
    char root_dir_path[50];
    sprintf(root_dir_path, "%s/%s_%dK_%dk_%dT_%dM_%dZP",parent_dir.c_str(), dataset_name.c_str(), tokenNum / 1000, k, T, doc_lim / 1000000, zoneMpSize);
    if (IsFileExist(root_dir_path)) {
        cout << "get the target root path" << endl;
    } else {
        cout << "Error! Target Root Dir not exist" << endl;
    }

    string str(root_dir_path);
    cout << "Root Dir: "<< str <<endl;
    return str;
}

string createRootDir(const string & parent_dir, const int &tokenNum, const int &k, const int &T, const int & doc_lim,  const int &zoneMpSize, const string & dataset_name){
    char root_dir_path[50];
    sprintf(root_dir_path,"%s/%s_%dK_%dk_%dT_%dM_%dZP",parent_dir.c_str(),dataset_name.c_str(),tokenNum/1000,k,T,doc_lim/1000000, zoneMpSize);
    if(!IsFileExist(root_dir_path)){
        mkdir(root_dir_path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        printf("%s Directory Made\n", root_dir_path);
    }   
    string str(root_dir_path);
    return str;
}

void getSonDir(const string &root_path, string &cw_dir, string &index_file, string &zonemap_dir) {
    cw_dir = root_path + "/compatWindows/";
    index_file = root_path + "/index.bin";
    zonemap_dir = root_path + "/zonemap/";

    // create directory for cw dir and zonemap_dir if they do not exist
    if(!IsFileExist(cw_dir.c_str())){
        printf("%s Directory No exist\n", cw_dir.c_str());
    }   
    if(!IsFileExist(zonemap_dir.c_str())){
        printf("%s Directory No exist\n", zonemap_dir.c_str());
    }
}

void createSonDir(const string& root_path, string & cw_dir, string & index_file, string& zonemap_dir){
    cw_dir = root_path+"/compatWindows/";
    index_file = root_path+"/index.bin";
    zonemap_dir = root_path+"/zonemap/";

    // create directory for cw dir and zonemap_dir if they do not exist
    if(!IsFileExist(cw_dir.c_str())){
        mkdir(cw_dir.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        printf("%s Directory Made\n", cw_dir.c_str());
    }   
    if(!IsFileExist(zonemap_dir.c_str())){
        mkdir(zonemap_dir.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        printf("%s Directory Made\n", zonemap_dir.c_str());
    }
}

void getScatteredSonDir(const string &scattered_dir, string &cw_dir, string &index_dir, string &zonemap_dir) {
    cw_dir = scattered_dir + "compatWindows/";
    index_dir = scattered_dir + "index/";
    zonemap_dir = scattered_dir + "zonemap/";
}

void getDocIndexSonDir(const string& root_path, string & docIndex_filePath, string & t_dirPath, string& docOfs_dirPath){
    docIndex_filePath = root_path+"/docIndex.bin";
    t_dirPath = root_path+"/t/";
    docOfs_dirPath = root_path+"/docOfs/";

    // create directory for cw dir and zonemap_dir if they do not exist
    if(!IsFileExist(t_dirPath.c_str())){
        printf("%s Directory No exist\n", t_dirPath.c_str());
    }
        
    if(!IsFileExist(docOfs_dirPath.c_str())){
        printf("%s Directory No exist\n", docOfs_dirPath.c_str());
    }
}

void createDocIndexSonDir(const string& root_path, string & docIndex_filePath, string & t_dirPath, string& docOfs_dirPath){
    docIndex_filePath = root_path+"/docIndex.bin";
    t_dirPath = root_path+"/t/";
    docOfs_dirPath = root_path+"/docOfs/";

    // create directory for cw dir and zonemap_dir if they do not exist
    if(!IsFileExist(t_dirPath.c_str())){
        mkdir(t_dirPath.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        printf("%s Directory Made\n", t_dirPath.c_str());
    }
        
    if(!IsFileExist(docOfs_dirPath.c_str())){
        mkdir(docOfs_dirPath.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        printf("%s Directory Made\n", docOfs_dirPath.c_str());
    }
}




void readDocInex(vector<unsigned long long> & doc_index, const string & docIndex_file){
    ifstream ifs(docIndex_file, ios::binary);
    unsigned long long offset;
    while (ifs.read((char *)&offset, sizeof(unsigned long long ))) {
        doc_index.emplace_back(offset);
    }
    ifs.close();

    printf("---------------Index of Documents Read-----------------\n");
}

void getDocContent(vector<int> & tokens, const int &doc_id, const vector<unsigned long long> &doc_index, const string & binFIle){
    ifstream ifs(binFIle, ios::binary);
    unsigned long long offset = doc_index[doc_id];
    ifs.seekg(offset, ios_base::beg);

    int size;
    ifs.read((char *)&size, sizeof(int));

    int tmp_token;
    for(int i =0;i<size;i++){
        ifs.read((char *)&tmp_token, sizeof(int));
        tokens.emplace_back(tmp_token);
    }
    ifs.close();
}

void getPassage(const unsigned & doc_id, const vector<unsigned long long> &doc_index, const string & file_path, const int l, const int r, vector<int> &passage){
    vector<int> doc;
    getDocContent(doc, doc_id, doc_index, file_path);
    assert(l<doc.size() && r-1<doc.size());
    passage.assign(doc.begin()+l, doc.begin()+r-1);
}