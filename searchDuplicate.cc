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
    // 写死, 用seed为0的随机种子生成的k个hash function
    unsigned int seed = 0;
    for (int i = 0; i < k; i++) generateHashFunc(seed, hashFunctions);
}

void loadIndexItem(int k, string index_file){
    printf("------------------Loading Index File------------------\n");
    indexArr = new IndexItem *[k];

    for (int i = 0; i < k; i++) {
        indexArr[i] = new IndexItem[wordNum];
    }

    ifstream inFile(index_file, ios::out | ios::binary);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < wordNum; j++) {
            inFile.read((char *)&indexArr[i][j], sizeof(IndexItem));
        }
    }
    inFile.close();
    printf("------------------Index File Loaded------------------\n");
}


int main(){
    int k = 10; // the number of hash functions
    string query_seq = "was a research scientist working for the U.S. Department of Agriculture, and his mother, Edwina Sunny Lynch (née Sundholm; 1919–2004), was an English language tutor. ";
    float theta = 0.9;
    
    prepareGlobalVariables(k);
    // load the IndexItem
    loadIndexItem(k, "indexFile");

    // Create query
    Query query(query_seq, theta, k);
    
    printf("指针（地址）的值为：OX%p\n",indexArr);
    
    vector<CW> duplicateCWs = query.getResult();

    for(auto const & cw : duplicateCWs){
        cw.display();
    }

    
    

}