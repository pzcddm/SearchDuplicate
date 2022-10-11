#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "util/nearDupSearch.hpp"
#include "util/cw.hpp"
#include "util/IO.hpp"

#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/indexItem.hpp"
#include "util/query.hpp"

using namespace std;


// global variables
IndexItem **indexArr;
vector<pair<int, int>> hashFunctions;
int wordNum;

void prepareGlobalVariables(int k){

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
    
    int max_k = 100; // the maximum number of hash functions
    wordNum = 64000;
    int k = 100;
    float theta = 0.9;
    const string cw_dir = "compatWindows/openwebtext/";
    const string indexFile = "index/indexOpenWebText.bin";
    const string tokSeqFile ="../GenerationExperiment/tokenizeSeq/gpt2-small-seq.bin";

    // load the tokenized sequences
    vector<vector<int>> tokenizedSeqs;
    loadBin(tokSeqFile,tokenizedSeqs);
    
    prepareGlobalVariables(max_k);
    // load the IndexItem
    loadIndexItem(max_k, indexFile);

    cout<<"tokenized seq Num"<< tokenizedSeqs[0].size() <<endl;
    for(auto const & tmp: tokenizedSeqs[0]){
        cout<<tmp<<endl;
    }
    if(tokenizedSeqs[0].size()<k){
        cout<<"error the tokenized sequence length is smaller than k!"<<endl;
    }

    // Create query
    unsigned int windowsNum = 0;
    Query query(tokenizedSeqs[0], theta, k, cw_dir);

    
    
    vector<CW> duplicateCWs = query.getResult(windowsNum);


    printf("Report Total Windows Num: %u\n", windowsNum);
    // // read words from source folder
    // string src_path = "./dataset/test_byte/";
    // string file_name = "test.bytes";
    // vector<string> files; // store file_path of each document in the given folder
    // loadFilesNameByBytes(file_name,src_path, files);

    // // getFiles(src_path, files);
    // for(auto const & cw : duplicateCWs){
    //     cout<<"Document name: "<< files[cw.T]<<endl;
    //     cw.display();
    // }

    cout<<"total founded passages amount: "<<reportPassagesNum(duplicateCWs)<<endl;
    cout<<"total founded intervals amount: "<< duplicateCWs.size() <<endl;

}