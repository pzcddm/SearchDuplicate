#pragma once

#include <locale>
#include <codecvt>
#include <tuple>

using namespace std;
int K = 16;
int tau = 1;
double theta = 0.8;
int MY_NAN = -999;
int INFTY = 0x7fffffff;
int INFTY_NEG = -999999;
auto s_ext = chrono::high_resolution_clock::now();
auto e_ext = chrono::high_resolution_clock::now();
auto dur_ext = chrono::duration_cast<chrono::microseconds>(e_ext - s_ext);

auto s_update = chrono::high_resolution_clock::now();
auto e_update = chrono::high_resolution_clock::now();
auto dur_update = chrono::duration_cast<chrono::microseconds>(e_update - s_update);
wstring s2ws(const string &str) {
    using convert_typeX = codecvt_utf8<wchar_t>;
    wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

string ws2s(const wstring &wstr) {
    using convert_typeX = codecvt_utf8<wchar_t>;
    wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

// Turn on timer
std::chrono::_V2::system_clock::time_point LogTime() {
    return chrono::high_resolution_clock::now();
}

// Turn off timer
double RepTime(const std::chrono::_V2::system_clock::time_point &start) {
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    return duration.count() / 1000000.0;
}

// The hash value function
inline int hval(const vector<pair<int, int>> &hf, int word, int kth_hash = 0) {
    return hf[kth_hash].first * word + hf[kth_hash].second;
}
// printInfo of 2-dim vector
void printInfo(vector<vector<int>> vs) {
    for (auto &item : vs) {
        for (int i = 0; i < item.size(); i++) {
            cout << item[i] << ", ";
        }
        cout << endl;
    }
}
// Generate random hash functions based on given seed
// ab: the output argument
void generateHashFunc(unsigned int seed, vector<pair<int, int>> &hf) {
    // TODO: knuth shuffling?
    // TODO: random_seed
    srand(seed);
    int a = 0;
    while (a == 0)
        a = rand();
    int b = rand();
    hf.emplace_back(a, b);
}

// Read stopwords from given file name
void readStopWords(const string &fileName, unordered_set<string> &stopWords) {
    // one per line
    string stopWord;

    ifstream file(fileName, ios::in);
    while (getline(file, stopWord))
        stopWords.insert(stopWord);
}

void strToTokens(string &str, const string &delimiter, vector<string> &res, vector<int> &offsets) {
    /*
    // Replace illegal chars
    for (int i = 0; i < str.length(); ++i) {
        str[i] = str[i] <= 0 || str[i] == '\n'? ' ': str[i];
    }
    */
    char *inputStr = strdup(str.c_str());
    int wordNum = 0;
    char *key = strtok(inputStr, delimiter.c_str());

    // Iterate each word
    while (key) {
        // Calculate start position
        int startPos = key - inputStr;
        int endPos = startPos + strlen(key);
        wordNum++;
        for (int p = 0; p < strlen(key); p++)
            key[p] = tolower(key[p]);
        offsets.push_back(startPos);
        res.push_back(key);
        key = strtok(0, delimiter.c_str());
    }
    delete[] inputStr;
}
void splitWord(string str, vector<int> &words) {
    string word = "";
    for (auto x : str) {
        if (x == ' ') {
            if (word != "") words.push_back(stoi(word));
            word = "";
        } else {
            word = word + x;
        }
    }
    if (word != "") words.push_back(stoi(word));
}
void gptToken2int(const string &filename, vector<int> &doc, vector<int> &offsets) {
    // Read from file ...
    ifstream datafile(filename, ios::binary);
    datafile.seekg(0, std::ios_base::end);
    int length = datafile.tellg();
    string docstr(length + 1, '\0');
    datafile.seekg(0);
    datafile.read(&docstr[0], length);
    splitWord(docstr, doc);
    for (int i = 0; i < doc.size(); i++) offsets.push_back(i);
    // cout << docstr << endl;
}
// Read from file and do preprocessing
//
// Input parameter:
//  1) filename: the file name
//  2) stopwords: stopwords that don't need to be considered
//
// Output parameter:
//  1) doc: contains the word id (from word string => word id)
//  2) ppos: contains peroid positions
//  3) word2id: the mapping from word to id
//  4) id2word: the mapping from id to word
//  5) id2maxFreq: the mapping from id to maximum frequency
//  6) id2mulId: if this word appears more than 1 time, it will have a unique mulId for each appearance

// return a hash value for a vector
size_t getVectorHash(const vector<int> &vec) {
    size_t seed = vec.size();
    for (auto x : vec) {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}



// check whether one file exists
bool is_file_exist(const string &fileName) {
    ifstream infile(fileName);
    return infile.good();
}


string loadFile(string file_name, int offset, int length) {
    ifstream src_file(file_name, ios::binary);
    src_file.seekg(offset);
    string content(length + 1, '\0');
    src_file.read(&content[0], length);
    return content;
}


// print out a vector
template <class T>
void printVec(const vector<T> &vec) {
    for (auto const &tmp : vec) {
        cout << tmp << ",";
    }
    cout << endl;
}