#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "util/nearDupSearch.hpp"
#include "util/cw.hpp"
#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/indexItem.hpp"
#include "util/query.hpp"

using namespace std;


// global variables
unordered_map<string, int> word2id;
IndexItem **indexArr;
vector<pair<int, int>> hashFunctions;
unordered_set<string> stopwords;
int wordNum;

void prepareGlobalVariables(int k){
    // read Stop words
    readStopWords("stopwords.txt", stopwords);
    // Load the map of word to int
    const string wiki_words_file_name = "wiki_test_words.txt";
    loadWord2id(wiki_words_file_name, word2id);
    wordNum = word2id.size(); // the number of tokens
    cout << "total token amount: "<<wordNum << endl;
    // 写死, 用seed为0~k-1的随机种子生成的k个hash function 
    for (int i = 0; i < k; i++) generateHashFunc(i, hashFunctions);
}

void loadIndexItem(int k, string index_file){
    printf("------------------Loading Index File------------------\n");
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::in | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(IndexItem));
            if(indexArr[i][j].offset == 1808608800ULL )
            {
                cout << i<< "   "<<j<<endl;
                cout << indexArr[i][j].windowsNum<<endl;
            }
        }
    }
    inFile.close();
    printf("------------------Index File Loaded------------------\n");
}


int main(){
    
    int max_k = 100; // the maximum number of hash functions

    int k = 40;
    string query_seq = "was a research scientist working for the U.S. Department of Agriculture, and his mother, Edwina Sunny Lynch (née Sundholm; 1919–2004), was an English language tutor. ";
    float theta = 0.9;
    
    prepareGlobalVariables(max_k);
    // load the IndexItem
    loadIndexItem(max_k, "indexFile");

    // Create query
    Query query(query_seq, theta, k);
    
    vector<CW> duplicateCWs = query.getResult();
    cout<<"total founded intervals amount: "<< duplicateCWs.size() <<endl;
    // read words from source folder
    // string src_path = "./py_script/1k_dir/";
    string src_path = "./dataset/test_byte/";
    string file_name = "test.bytes";
    vector<string> files; // store file_path of each document in the given folder
    loadFilesNameByBytes(file_name,src_path, files);

    // getFiles(src_path, files);
    for(auto const & cw : duplicateCWs){
        cout<<"Document name: "<< files[cw.T]<<endl;
        cw.display();
    }

}