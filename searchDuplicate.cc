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
        }
    }
    inFile.close();
    printf("------------------Index File Loaded------------------\n");
}

int reportPassagesNum(const vector<CW> & duplicateCWs){
    int pasNum = 0;
    CW tmp_cw(0,-1,-1,-1);
    for(auto const & cw : duplicateCWs){
        if(tmp_cw.intersected(cw)){
            tmp_cw.merge(cw);
        }else{
            pasNum ++;
            tmp_cw = cw;
        }
    }
    return pasNum;
} 
int main(){
    
    int max_k = 40; // the maximum number of hash functions

    int k = 20;
    // string query_seq = "The San Saba Mission was established in April 1757 near the site of present day Menard, Texas.  Three miles away, a military post, Presidio San Luis de las Amarillas, was established at the same time to protect the Mission.";
    string query_seq = "Virginia, but since his parents had moved to Walnut Creek, California, he stayed with his friend Toby Keeler for a while. He decided to move to the city of Philadelphia and enroll at the Pennsylvania Academy of Fine Arts,";
    float theta = 0.7;
    const string cw_dir = "compatWindows/test/";
    prepareGlobalVariables(max_k);
    // load the IndexItem
    loadIndexItem(max_k, "indexFileTest.bin");

    // Create query
    unsigned int windowsNum = 0;
    Query query(query_seq, theta, k, cw_dir);
    
    vector<CW> duplicateCWs = query.getResult(windowsNum);

    printf("Report Total Windows Num: %u\n", windowsNum);
    // read words from source folder
    string src_path = "./dataset/test_byte/";
    string file_name = "test.bytes";
    vector<string> files; // store file_path of each document in the given folder
    loadFilesNameByBytes(file_name,src_path, files);

    // getFiles(src_path, files);
    for(auto const & cw : duplicateCWs){
        cout<<"Document name: "<< files[cw.T]<<endl;
        cw.display();
    }

    cout<<"total founded passages amount: "<<reportPassagesNum(duplicateCWs)<<endl;
    cout<<"total founded intervals amount: "<< duplicateCWs.size() <<endl;

}