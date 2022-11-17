#pragma once

#include <locale>
#include <codecvt>
#include "ds.hpp"
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

void generateSubSketch(vector<int> &sub_sketch, unordered_map<size_t, int> &bucket2index) {
    size_t seed = getVectorHash(sub_sketch);
    if (bucket2index.find(seed) == bucket2index.end()) {
        int id = bucket2index.size();
        bucket2index[seed] = id;
    }
}

// api for words
void getWords(const vector<string> &files, vector<string> &words, const unordered_set<string> &stopWords) {
    int id_size = 0;
    unordered_map<string, int> word2id;
    for (auto &file : files) {
        // Read from file ...
        ifstream datafile(file, ios::binary);
        datafile.seekg(0, std::ios_base::end);
        int length = datafile.tellg();
        string docstr(length + 1, '\0');
        datafile.seekg(0);
        datafile.read(&docstr[0], length);

        // Make the doc to tokens
        const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";
        vector<string> tokens;
        vector<int> tokensOffsets;

        strToTokens(docstr, delim, tokens, tokensOffsets);
        for (int i = 0; i < tokens.size(); i++) {
            string word = tokens[i];
            if (stopWords.find(word) != stopWords.end())
                continue;
            // If a new word, add to word2id
            if (word2id.find(word) == word2id.end()) {
                word2id[word] = id_size;
                id_size += 1;
                words.emplace_back(word);
            }
        }
    }
    cout << "word2id size is: " << word2id.size() << endl;
    cout << "words size is: " << words.size() << endl;
}
void writeWords(const string file_name, vector<string> &words) {
    ofstream fout;
    fout.open(file_name, ios::out);
    for (auto &word : words) {
        fout << word << endl;
    }
    fout.close();
}

void loadWord2id(string file_name, unordered_map<string, int> &word2id) {
    ifstream file(file_name, ios::in);
    string word;
    int index = 0;
    while (getline(file, word)) {
        word2id[word] = index;
        index += 1;
    }
    file.close();
}

void getIntersection(const vector<int> &v1, const vector<int> &v2, vector<int> &ret) {
    unordered_set<int> s1;
    for (auto &item : v1) {
        s1.insert(item);
    }
    for (auto &item : v2) {
        if (s1.find(item) != s1.end()) {
            ret.push_back(item);
        }
    }
    sort(ret.begin(), ret.end());
}
void getUnion(const vector<int> &v1, const vector<int> &v2, vector<int> &ret) {
    unordered_set<int> s;
    for (auto &item : v1) {
        s.insert(item);
    }
    for (auto &item : v2) {
        s.insert(item);
    }
    vector<int> temp_ret;
    for (auto &item : s) {
        ret.push_back(item);
    }
}

// check whether one file exists
bool is_file_exist(const string &fileName) {
    ifstream infile(fileName);
    return infile.good();
}

void getSketches(const vector<vector<int>> docs, vector<vector<int>> &sketches, vector<pair<int, int>> &hf) {
    for (int docid = 0; docid < docs.size(); docid++) {
        vector<int> hashes;
        for (int pos = 0; pos < docs[docid].size(); pos++) {
            int hash = hval(hf, docs[docid][pos]);
            hashes.push_back(hash);
        }
        sort(hashes.begin(), hashes.end());
        for (int j = 0; j < K; j++) {
            sketches[docid].push_back(hashes[j]);
        }
    }
}
// find exactly the same one
void clustering(const vector<string> &files, const vector<vector<int>> &sketches, unordered_map<size_t, vector<string>> &cluster) {
    for (int i = 0; i < sketches.size(); i++) {
        size_t v_hash = getVectorHash(sketches[i]);
        if (cluster.find(v_hash) == cluster.end()) {
            // not exist such a key
            cluster[v_hash].emplace_back(files[i]);
        } else {
            cluster[v_hash].push_back(files[i]);
        }
    }
}
// find pair whose jaccard similarity is over one given threshold
void getTokens(const string file_name, int offset, int length, vector<string> &tokens) {
    ifstream file(file_name, ios::binary);
    file.seekg(offset);
    string content(length + 1, '\0');
    file.read(&content[0], length);
    const string delim = "\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~\n\t\r\x0b\x0c ,.!?;!";
    vector<int> tokensOffsets;
    // cout << "orginal doc info is: " << content << endl << endl;
    strToTokens(content, delim, tokens, tokensOffsets);
}
float getJacWoutStem(docInfo &d1, docInfo &d2, vector<string> &files) {
    string f1 = files[d1.this_id];
    int off1 = d1.this_offset;
    int len1 = d1.this_length;
    string f2 = files[d2.this_id];
    int off2 = d2.this_offset;
    int len2 = d2.this_length;
    vector<string> w1;
    vector<string> w2;
    getTokens(f1, off1, len1, w1);
    getTokens(f2, off2, len2, w2);
    unordered_set<string> w1_set;
    unordered_set<string> w2_set;
    for (auto &w : w1) {
        w1_set.insert(w);
    }
    int inter_words = 0;
    for (auto &w : w2) {
        w2_set.insert(w);
    }
    for (auto &w : w1_set) {
        if (w2_set.find(w) != w2_set.end()) {
            inter_words++;
        }
    }
    return (inter_words + 0.0) / (w1_set.size() + w2_set.size() - inter_words);
}

void findMaxJacAndIndex(vector<float> &jac_row, int &index, float &max_jac) {
    max_jac = -1.0;
    index = 0;
    for (int i = 0; i < jac_row.size(); i++) {
        if (jac_row[i] > max_jac) {
            // cout << "current jac is: " << jac_row[i] << endl;
            max_jac = jac_row[i];
            index = i;
        }
    }
}
void getTokensByStr(string &str, vector<string> &res) {
    const string delimiter = "\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~\n\t\r\x0b\x0c ,.!?;!";
    char *inputStr = strdup(str.c_str());
    int wordNum = 0;
    char *key = strtok(inputStr, delimiter.c_str());

    // Iterate each word
    while (key) {
        // Calculate start position
        int startPos = key - inputStr;
        int endPos = startPos + strlen(key);
        for (int p = 0; p < strlen(key); p++)
            key[p] = tolower(key[p]);
        res.push_back(key);
        key = strtok(0, delimiter.c_str());
    }
    delete[] inputStr;
}
float getJacStrDoc(docInfo &d1, docInfo &d2, vector<string> &docs) {
    int d1_id = d1.this_id;
    int d1_off = d1.this_offset;
    int d1_len = d1.this_length;
    int d2_id = d2.this_id;
    int d2_off = d2.this_offset;
    int d2_len = d2.this_length;
    string d1_con = docs[d1_id].substr(d1_off, d1_len);
    string d2_con = docs[d2_id].substr(d2_off, d2_len);
    vector<string> t1;
    vector<string> t2;
    getTokensByStr(d1_con, t1);
    getTokensByStr(d2_con, t2);
    unordered_set<string> t1_set;
    unordered_set<string> t2_set;
    for (auto &w : t1) {
        t1_set.insert(w);
    }
    int inter_words = 0;
    for (auto &w : t2) {
        t2_set.insert(w);
    }
    for (auto &w : t1_set) {
        if (t2_set.find(w) != t2_set.end()) {
            inter_words++;
        }
    }
    return (inter_words + 0.0) / (t1_set.size() + t2_set.size() - inter_words);
}
float getJacByInt(docInfo &d1, docInfo &d2, vector<vector<int>> &docs) {
    int d1_id = d1.this_id;
    int d1_off = d1.this_offset;
    int d1_len = d1.this_length;
    int d2_id = d2.this_id;
    int d2_off = d2.this_offset;
    int d2_len = d2.this_length;

    unordered_set<int> t1_set;
    unordered_set<int> t2_set;
    for (int i = 0; i < d1_len; i++) {
        t1_set.insert(docs[d1_id][d1_off + i]);
    }
    for (int i = 0; i < d2_len; i++) {
        t2_set.insert(docs[d2_id][d2_off + i]);
    }
    int inter_words = 0;
    for (auto &w : t1_set) {
        if (t2_set.find(w) != t2_set.end()) {
            inter_words++;
        }
    }
    return (inter_words + 0.0) / (t1_set.size() + t2_set.size() - inter_words);
}
// with period postion version copied from github project RangeAllign first commit
void SketchGeneration(vector<CompositeWindow> &res_cws, const vector<int> &doc, const vector<int> &ppos, const vector<pair<int, int>> &hf) {
    vector<pair<int, int>> words_hash_pos;
    for (int word_pos = 0; word_pos < doc.size(); word_pos++) {
        words_hash_pos.emplace_back(hval(hf, doc[word_pos]), word_pos);
    }

    // first sort by hash value and second by position
    sort(words_hash_pos.begin(), words_hash_pos.end(), [](const pair<int, int> &p1, const pair<int, int> &p2) {
        if (p1.first < p2.first)
            return true;
        else if (p1.first > p2.first)
            return false;
        else
            return p1.second < p2.second;
    });

    // build the occurrence table, one double linked list
    vector<Node> occurrences(doc.size());
    int prev_hv = words_hash_pos.front().first + 1;
    int prev_pos = -1;
    for (auto it = words_hash_pos.begin(); it != words_hash_pos.end(); ++it) {
        int hv = it->first;
        int pos = it->second;
        // cout << "pos: " << pos << " hash " << hv << endl;
        if (hv == prev_hv) {
            occurrences[pos].prev = prev_pos;
            occurrences[prev_pos].next = pos;
        } else {
            if (prev_pos != -1)
                occurrences[prev_pos].next = INFTY;

            prev_hv = hv;
            occurrences[pos].prev = INFTY_NEG;
        }
        prev_pos = pos;
    }
    occurrences[prev_pos].next = INFTY;

    // maintain the skip list using double linked list trick
    int doc_size = doc.size();
    vector<Node> linkedlist;

    Node HEAD(MY_NAN, 0);
    Node TAIL(doc_size - 1, MY_NAN);
    int HEAD_ID = -1;
    int TAIL_ID = doc_size;

    for (int word_pos = 0; word_pos < doc.size(); word_pos++)
        linkedlist.emplace_back(word_pos - 1, word_pos + 1);

    // keep the neighbors when visiting
    vector<Node> neighbors(doc.size());
    for (auto rit = words_hash_pos.rbegin(); rit != words_hash_pos.rend(); ++rit) {
        int pos = rit->second;
        // first record the two neighbors of pos in current linked list
        neighbors[pos].next = linkedlist[pos].next;
        neighbors[pos].prev = linkedlist[pos].prev;

        // next update the linked list by removing pos
        int next = linkedlist[pos].next;
        int prev = linkedlist[pos].prev;

        if (next != TAIL_ID)
            linkedlist[next].prev = prev;
        else
            TAIL.prev = prev;

        if (prev != HEAD_ID)
            linkedlist[prev].next = next;
        else
            HEAD.next = next;
    }
    // initialize a linked list for D;
    vector<Node> D(doc.size() + 1);

    // cout << "visiting words" << endl;
    // now, visiting in ascending order
    vector<Node> skiplist(doc.size());
    Node SKIP_HEAD(MY_NAN, TAIL_ID);
    Node SKIP_TAIL(HEAD_ID, MY_NAN);
    for (auto it = words_hash_pos.begin(); it != words_hash_pos.end(); ++it) {
        // cout << "pos: " << it->second << " hash " << it->first << endl;
        int pos = it->second;
        // first insert into the skip list
        int next = neighbors[pos].next;
        int prev = neighbors[pos].prev;

        skiplist[pos].next = next;
        skiplist[pos].prev = prev;

        if (next != TAIL_ID)
            skiplist[next].prev = pos;
        else
            SKIP_TAIL.prev = pos;

        if (prev != HEAD_ID)
            skiplist[prev].next = pos;
        /*
        else
            SKIP_HEAD.next = pos;
        */
        // next generate sketches
        int x = pos;
        vector<int> L;
        while (x != TAIL_ID && L.size() < K) {
            x = skiplist[x].next;
            if (x == TAIL_ID || occurrences[x].prev < pos)
                L.push_back(x);
        }

        for (auto rit = L.rbegin(); rit != L.rend(); ++rit) {
            int cx = *rit;
            int cy;
            if (cx != TAIL_ID)
                cy = skiplist[cx].prev;
            else
                cy = SKIP_TAIL.prev;

            int D_size = 0;
            int HEAD_REF = MY_NAN;

            while (cy != HEAD_ID && D_size <= K && occurrences[cy].next != pos) {
                if (cy == pos || occurrences[cy].next > cx) {
                    D[cy].next = HEAD_REF;
                    D[cy].prev = MY_NAN;
                    if (HEAD_REF != MY_NAN)
                        D[HEAD_REF].prev = cy;
                    HEAD_REF = cy;
                    D_size += 1;
                } else if (occurrences[cy].next == cx)
                    break;
                else if (occurrences[cy].next < cx) {
                    int del = occurrences[cy].next;
                    if (HEAD_REF == del)
                        HEAD_REF = D[del].next;

                    if (D[del].next != MY_NAN) {
                        int del_next = D[del].next;
                        D[del_next].prev = D[del].prev;
                    }

                    if (D[del].prev != MY_NAN) {
                        int del_prev = D[del].prev;
                        D[del_prev].next = D[del].next;
                    }

                    D[cy].next = HEAD_REF;
                    D[cy].prev = MY_NAN;
                    if (HEAD_REF != MY_NAN)
                        D[HEAD_REF].prev = cy;
                    HEAD_REF = cy;
                }

                if (D_size == K) {
                    int ll = skiplist[cy].prev + 1;
                    int lr = cy;
                    int rr = cx - 1;
                    int index = 0;
                    int left_l = 0;
                    int right_r = 0;
                    while (index < ppos.size()) {
                        if (ppos[index] <= lr && ll <= ppos[index]) {
                            left_l = ppos[index];
                            break;
                        }
                        index++;
                    }
                    int idx = HEAD_REF;
                    vector<int> positions;
                    vector<int> sketch;
                    int rl = 0;
                    while (idx != MY_NAN) {
                        positions.push_back(idx);
                        sketch.push_back(hval(hf, doc[idx]));
                        rl = idx;
                        idx = D[idx].next;
                    }
                    while (index < ppos.size()) {
                        if (ppos[index] <= rr && rl <= ppos[index]) {
                            right_r = ppos[index];
                            break;
                        }
                        index++;
                    }
                    index = 0;
                    if (index != ppos.size()) {
                        if ((rl - cy) >= tau) {
                            // res_cws.emplace_back(left_l, cy, rl, right_r);
                            res_cws.emplace_back(skiplist[cy].prev + 1, cy, rl, cx - 1);
                            for (int i = 0; i < K; i++) {
                                res_cws.back().positions.push_back(positions[i]);
                                res_cws.back().sketch.push_back(sketch[i]);
                            }
                        }
                    }
                }
                cy = skiplist[cy].prev;
            }
        }
    }
}

// usable getSkecth interface
void getSketch(int pos, const vector<int> &doc, vector<Node> &occurrences, vector<Node> &neighbors, vector<Node> &skiplist, Node &SKIP_TAIL, vector<Node> &D, const vector<int> &ppos, const vector<pair<int, int>> &hf, vector<CompositeWindow> &res_cws) {
    // first insert into the skip list
    cout << "line 990 in util.hpp execute getSketch function here" << endl;
    int HEAD_ID = -1;
    int TAIL_ID = doc.size();
    int next = neighbors[pos].next;
    int prev = neighbors[pos].prev;

    skiplist[pos].next = next;
    skiplist[pos].prev = prev;

    if (next != TAIL_ID)
        skiplist[next].prev = pos;
    else
        SKIP_TAIL.prev = pos;

    if (prev != HEAD_ID)
        skiplist[prev].next = pos;
    /*
    else
        SKIP_HEAD.next = pos;
    */
    // next generate sketches
    int x = pos;
    vector<int> L;
    while (x != TAIL_ID && L.size() < K) {
        x = skiplist[x].next;
        if (x == TAIL_ID || occurrences[x].prev < pos)
            L.push_back(x);
    }

    for (auto rit = L.rbegin(); rit != L.rend(); ++rit) {
        int cx = *rit;
        int cy;
        if (cx != TAIL_ID)
            cy = skiplist[cx].prev;
        else
            cy = SKIP_TAIL.prev;

        int D_size = 0;
        int HEAD_REF = MY_NAN;

        while (cy != HEAD_ID && D_size <= K && occurrences[cy].next != pos) {
            if (cy == pos || occurrences[cy].next > cx) {
                D[cy].next = HEAD_REF;
                D[cy].prev = MY_NAN;
                if (HEAD_REF != MY_NAN)
                    D[HEAD_REF].prev = cy;
                HEAD_REF = cy;
                D_size += 1;
            } else if (occurrences[cy].next == cx)
                break;
            else if (occurrences[cy].next < cx) {
                int del = occurrences[cy].next;
                if (HEAD_REF == del)
                    HEAD_REF = D[del].next;

                if (D[del].next != MY_NAN) {
                    int del_next = D[del].next;
                    D[del_next].prev = D[del].prev;
                }

                if (D[del].prev != MY_NAN) {
                    int del_prev = D[del].prev;
                    D[del_prev].next = D[del].next;
                }

                D[cy].next = HEAD_REF;
                D[cy].prev = MY_NAN;
                if (HEAD_REF != MY_NAN)
                    D[HEAD_REF].prev = cy;
                HEAD_REF = cy;
            }

            if (D_size == K) {
                int ll = skiplist[cy].prev + 1;
                int lr = cy;
                int rr = cx - 1;

                int index = 0;
                int left_l = 0;
                int right_r = 0;
                while (index < ppos.size()) {
                    if (ppos[index] <= lr && ll <= ppos[index]) {
                        left_l = ppos[index];
                        break;
                    }
                    index++;
                }

                int idx = HEAD_REF;
                vector<int> positions;
                vector<int> sketch;
                int rl = 0;
                while (idx != MY_NAN) {
                    positions.push_back(idx);
                    sketch.push_back(hval(hf, doc[idx]));
                    rl = idx;
                    idx = D[idx].next;
                }
                while (index < ppos.size()) {
                    if (ppos[index] <= rr && rl <= ppos[index]) {
                        right_r = ppos[index];
                        break;
                    }
                    index++;
                }
                index = -1;
                cout << "tau is :" << tau << endl;
                if (true) {
                    if ((rl - cy) >= tau) {
                        // res_cws.emplace_back(left_l, cy, rl, right_r);
                        res_cws.emplace_back(skiplist[cy].prev + 1, cy, rl, cx - 1);
                        sort(sketch.begin(), sketch.end());
                        for (int i = 0; i < K; i++) {
                            res_cws.back().positions.push_back(positions[i]);
                            res_cws.back().sketch.push_back(sketch[i]);
                            cout << "generate sketch here" << endl;
                        }
                    }
                }
            }
            cy = skiplist[cy].prev;
        }
    }
}
// back up for former used getSketch interface
// currently used getSkecth interface
// bool value getRet : if it is False, just insert value into the skiplist
/*
void getSketch(vector<int> offset, int docid, bool getRet, int pos, const vector<int> &doc, vector<Node> &occurrences, vector<Node> &neighbors,  vector<Node> &skiplist, Node &SKIP_TAIL, vector<Node> &D, const vector<int> &ppos, const vector<pair<int, int>> &hf, vector<CompactWindow> &cws){
    // first insert into the skip list
    int HEAD_ID = -1;
    int TAIL_ID = doc.size();
    int next = neighbors[pos].next;
    int prev = neighbors[pos].prev;
    int max_hash_value = hval(hf, doc[pos]);
    skiplist[pos].next = next;
    skiplist[pos].prev = prev;

    if (next != TAIL_ID)
        skiplist[next].prev = pos;
    else
        SKIP_TAIL.prev = pos;

    if (prev != HEAD_ID)
        skiplist[prev].next = pos;
    if (not getRet) return ;
    // next generate sketches
    int x = pos;
    vector<int> L;
    while (x != TAIL_ID && L.size() < K)
    {
        x = skiplist[x].next;
        if (x == TAIL_ID || occurrences[x].prev < pos)
            L.push_back(x);
    }

    for (auto rit = L.rbegin(); rit != L.rend(); ++rit)
    {
        int cx = *rit;
        int cy;
        if (cx != TAIL_ID)
            cy = skiplist[cx].prev;
        else
            cy = SKIP_TAIL.prev;

        int D_size = 0;
        int HEAD_REF = MY_NAN;

        while (cy != HEAD_ID && D_size <= K && occurrences[cy].next != pos)
        {
            if (cy == pos || occurrences[cy].next > cx)
            {
                D[cy].next = HEAD_REF;
                D[cy].prev = MY_NAN;
                if (HEAD_REF != MY_NAN)
                    D[HEAD_REF].prev = cy;
                HEAD_REF = cy;
                D_size += 1;
            }
            else if (occurrences[cy].next == cx)
                break;
            else if (occurrences[cy].next < cx)
            {
                int del = occurrences[cy].next;
                if (HEAD_REF == del)
                    HEAD_REF = D[del].next;

                if (D[del].next != MY_NAN)
                {
                    int del_next = D[del].next;
                    D[del_next].prev = D[del].prev;
                }

                if (D[del].prev != MY_NAN)
                {
                    int del_prev = D[del].prev;
                    D[del_prev].next = D[del].next;
                }

                D[cy].next = HEAD_REF;
                D[cy].prev = MY_NAN;
                if (HEAD_REF != MY_NAN)
                    D[HEAD_REF].prev = cy;
                HEAD_REF = cy;
            }

            if (D_size == K)
            {
                int ll = skiplist[cy].prev + 1;
                int lr = cy;
                int rr = cx - 1;
                int index = 0;
                int left_l = 0;
                int right_r = 0;
                while (index < ppos.size()){
                    if (ppos[index] <= lr && ll <= ppos[index]){
                        left_l = ppos[index];
                        break;
                    }
                    index++;
                }
                int idx = HEAD_REF;
                vector<int> positions;
                vector<int> sketch;
                int rl = 0;
                while (idx != MY_NAN)
                {
                    positions.push_back(idx);
                    sketch.push_back(hval(hf, doc[idx]));
                    rl = idx;
                    idx = D[idx].next;
                }
                while (index < ppos.size()){
                    if (ppos[index] <= rr && rl <= ppos[index]){
                        right_r = ppos[index];
                        break;
                    }
                    index++;
                }
                index = 0;
                if (index != ppos.size()){
                    if ((rl - cy) >= tau){
                        //res_cws.emplace_back(left_l, cy, rl, right_r);
                        cws.emplace_back(offset[skiplist[cy].prev + 1], offset[cy], offset[rl], offset[cx - 1]);
                        //sort(sketch.begin(), sketch.end());
                        cws.back().max_hv = max_hash_value;
                        cws.back().doc_index = docid;
                        for (int i = 0; i < sketch.size(); i++){
                            cws.back().sketch.push_back(sketch[i]);
                        }
                    }
                }
            }
            cy = skiplist[cy].prev;
        }
    }
}
*/

int getDiff(const vector<int> &a, const vector<int> &b) {
    int diff = 0;
    unordered_set<int> set_a;
    for (auto &item : a) {
        set_a.insert(item);
    }
    for (auto &item : b) {
        if (set_a.find(item) == set_a.end()) {
            diff++;
        }
    }
    return diff;
}
void getChange(vector<vector<int>> &ske_ret, int &total) {
    vector<int> pre_sketch = ske_ret[0];
    for (int i = 1; i < ske_ret.size(); i++) {
        vector<int> cur_sketch = ske_ret[i];
        int diff = getDiff(pre_sketch, cur_sketch);
        if (diff > 1) {
            // printInfo(ske_ret);
            total++;
        }
        pre_sketch = cur_sketch;
    }
}
void getOverlapRet(vector<CompactWindow> &vcw1, vector<CompactWindow> &vcw2, vector<plaInfo> &ret) {
    unordered_map<size_t, vector<int>> cw2id;
    for (int i = 0; i < vcw2.size(); i++) {
        size_t key = getVectorHash(vcw2[i].sketch);
        if (cw2id.find(key) == cw2id.end()) {
            // not found, initialize it
            vector<int> value;
            value.push_back(i);
            cw2id[key] = value;
        } else {
            cw2id[key].push_back(i);
        }
    }
    for (auto &cw : vcw1) {
        size_t key = getVectorHash(cw.sketch);
        if (cw2id.find(key) != cw2id.end()) {
            for (auto &index : cw2id[key]) {
                ret.emplace_back(cw.doc_index, vcw2[index].doc_index, cw.beg_range, cw.end_range, vcw2[index].beg_range, vcw2[index].end_range);
            }
        }
    }
}
void getSubstr(const string doc_name, const int offset, const int length, string &content) {
    ifstream datafile(doc_name, ios::binary);
    datafile.seekg(0, std::ios_base::end);
    string docstr(length + 1, '\0');
    datafile.read(&content[0], length);
}
void getAllStr(const string doc_name, string &content) {
    ifstream datafile(doc_name, ios::binary);
    datafile.seekg(0, std::ios_base::end);
    int length = datafile.tellg();
    content.resize(length + 1, '\0');
    datafile.seekg(0);
    datafile.read(&content[0], length);
}
bool isInInterval(docInfo &di, vector<vector<pair<int, int>>> &intervals) {
    int id = di.this_id;
    int start = di.this_offset;
    int end = start + di.this_length;
    if (intervals[id].size() == 0) return false;
    for (auto &interval : intervals[id]) {
        int s = interval.first;
        int e = interval.second;
        // inside
        if (s <= start && end <= e) return true;
    }
    return false;
}
// check whether it is one repeated result
bool isDuplicate(vector<docInfo> &di_ret, vector<vector<pair<int, int>>> &intervals) {
    for (auto &di : di_ret) {
        if (!isInInterval(di, intervals)) return false;
    }
    return true;
}

void extendRet(vector<string> &files, vector<docInfo> &to_extend_ret, vector<docInfo> &extend_ret) {
    vector<int> doc_ids;
    vector<int> start_pos;
    vector<int> end_pos;
    vector<string> docs;
    for (auto &item : to_extend_ret) {
        doc_ids.push_back(item.this_id);
        start_pos.push_back(item.this_offset - 1);
        end_pos.push_back(item.this_offset + item.this_length + 1);
        string doc_con;
        getAllStr(files[item.this_id], doc_con);
        docs.push_back(doc_con);
    }
    while (1) {
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++) {
            int si = start_pos[i];
            int sni = start_pos[i + 1];
            if (si < 0 || sni < 0 || docs[i][si] != docs[i + 1][sni]) {
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++) {
            start_pos[i] = start_pos[i] - 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++) {
        start_pos[i] = start_pos[i] + 1;
    }
    while (1) {
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++) {
            int ei = end_pos[i];
            int eni = end_pos[i + 1];
            if (ei >= docs[i].size() || eni >= docs[i + 1].size() || docs[i][ei] != docs[i + 1][eni]) {
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++) {
            end_pos[i] = end_pos[i] + 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++) {
        end_pos[i] = end_pos[i] - 1;
    }
    for (int i = 0; i < doc_ids.size(); i++) {
        extend_ret.emplace_back(doc_ids[i], start_pos[i], end_pos[i] - start_pos[i] + 1);
    }
}
void updateInterval(vector<docInfo> &extend_ret, vector<vector<pair<int, int>>> &intervals) {
    for (auto &di : extend_ret) {
        int id = di.this_id;
        pair<int, int> p = make_pair(di.this_offset, di.this_offset + di.this_length - 1);
        intervals[id].push_back(p);
    }
}
string loadFile(string file_name, int offset, int length) {
    ifstream src_file(file_name, ios::binary);
    src_file.seekg(offset);
    string content(length + 1, '\0');
    src_file.read(&content[0], length);
    return content;
}
// refine result, keep only exactly match substring
void refineRet(vector<string> &files, vector<vector<docInfo>> temp_to_refine_ret, vector<vector<docInfo>> &final_ret, vector<vector<pair<int, int>>> &intervals) {
    hash<string> hasher;
    unordered_map<size_t, vector<int>> um;
    for (auto &temp : temp_to_refine_ret) {
        um.clear();
        for (int i = 0; i < temp.size(); i++) {
            docInfo di = temp[i];
            string content = loadFile(files[di.this_id], di.this_offset, di.this_length);
            size_t con_hash = hasher(content);
            um[con_hash].emplace_back(i);
        }
        for (auto &item : um) {
            vector<docInfo> temp_ret;
            if (item.second.size() >= 2) {
                for (auto &index : item.second) {
                    temp_ret.push_back(temp[index]);
                }
                if (isDuplicate(temp_ret, intervals)) {
                    // cout << "subset ske found here" << endl;
                    continue;
                }
                cout << "item first: " << item.first << endl;
                cout << "**************same cluster wout extension***********************" << endl;
                for (auto &di : temp_ret) {
                    int docid = di.this_id;
                    int off = di.this_offset;
                    int len = di.this_length;
                    string content = loadFile(files[docid], off, len);
                    cout << "-----------------------------------------" << endl;
                    cout << content << endl;
                    cout << "its hash value is: " << hasher(content) << endl;
                    cout << "-----------------------------------------" << endl;
                }
                cout << "**************same cluster wout extension***********************" << endl;
                vector<docInfo> extend_ret;
                extendRet(files, temp_ret, extend_ret);
                final_ret.push_back(extend_ret);
                updateInterval(extend_ret, intervals);
            }
        }
    }
}
size_t getPairHash(pair<int, int> &p) {
    vector<int> v;
    v.push_back(p.first);
    v.push_back(p.second);
    return getVectorHash(v);
}

/*
//get the final result
void getFinalRet(vector<string> &files, vector<vector<docInfo>> &temp_to_extend_ret){
    for (auto &temp: temp_to_extend_ret){

    }
}
*/
void writeSize_t(vector<size_t> &keys, string file_name = "keys.txt") {
    ofstream fout;
    fout.open(file_name, ios::out);
    for (auto &item : keys) {
        fout << item << endl;
    }
    fout.close();
}

void loadSize_t(vector<size_t> &keys, string file_name = "keys.txt") {
    ifstream fin;
    fin.open(file_name, ios::in);
    size_t key;
    while (fin.good()) {
        fin >> key;
        keys.push_back(key);
    }
    keys.pop_back();
    fin.close();
}

void writeVector(vector<vector<docInfo>> &values, string file_name = "values.txt") {
    ofstream fout;
    fout.open(file_name, ios::out);
    for (auto &item : values) {
        for (int i = 0; i < item.size(); i++) {
            fout << item[i].this_id << " " << item[i].this_offset << " " << item[i].this_length << " ";
        }
        fout << endl;
    }
    fout.close();
}
void savePairRet(vector<pair<docInfo, docInfo>> &pair_ret, string file_name = "pair_ret.txt") {
    ofstream fout;
    fout.open(file_name, ios::out);
    for (auto &item : pair_ret) {
        fout << item.first.this_id << " " << item.first.this_offset << " " << item.first.this_length << " ";
        fout << item.second.this_id << " " << item.second.this_offset << " " << item.second.this_length << endl;
    }
    fout.close();
}

void loadVector(vector<vector<docInfo>> &values, string file_name = "values.txt") {
    ifstream fin;
    fin.open(file_name, ios::in);
    string all_info;
    while (getline(fin, all_info)) {
        vector<int> words;
        // cout << all_info << endl;
        splitWord(all_info, words);
        vector<docInfo> temp;
        for (int i = 0; i < words.size(); i += 3) {
            int docid = words[i];
            int this_offset = words[i + 1];
            int this_length = words[i + 2];
            temp.emplace_back(docid, this_offset, this_length);
        }
        values.push_back(temp);
    }
    fin.close();
}
bool compareDocInfo(docInfo &a, docInfo &b) {
    if (a.this_id != b.this_id) return false;
    if (a.this_offset != b.this_offset) return false;
    if (a.this_length != b.this_length) return false;
    return true;
}
void checkDocInfo(vector<vector<docInfo>> &v1, vector<vector<docInfo>> &v2) {
    if (v1.size() != v2.size()) {
        cout << "unmatch " << endl;
        return;
    }
    for (int i = 0; i < v1.size(); i++) {
        vector<docInfo> d1 = v1[i];
        vector<docInfo> d2 = v2[i];
        if (d1.size() != d2.size()) {
            cout << "unmatch " << endl;
            return;
        }
        for (int j = 0; j < d1.size(); j++) {
            if (!compareDocInfo(d1[j], d2[j])) {
                cout << "unmatch " << endl;
                return;
            }
        }
    }
    cout << "all match" << endl;
}
void checkRet(vector<vector<docInfo>> &d, vector<vector<docInfo>> &save_d) {
    int total_pair = 0;
    for (int i = 0; i < d.size(); i++) {
        vector<docInfo> di = d[i];
        vector<docInfo> to_save;
        unordered_set<int> docids;
        for (int j = 0; j < di.size(); j++) {
            int id = di[j].this_id;
            if (docids.find(id) == docids.end()) {
                docids.insert(id);
                to_save.push_back(di[j]);
            }
        }
        if (docids.size() >= 2) {
            total_pair++;
            save_d.push_back(to_save);
        }
    }
    cout << "total pair is: " << total_pair << endl;
}
bool isSame(docInfo &d1, docInfo &d2, vector<string> &docs) {
    int d1_id = d1.this_id;
    int d1_off = d1.this_offset;
    int d1_len = d1.this_length;
    int d2_id = d2.this_id;
    int d2_off = d2.this_offset;
    int d2_len = d2.this_length;
    string d1_con = docs[d1_id].substr(d1_off, d1_len);
    string d2_con = docs[d2_id].substr(d2_off, d2_len);
    return d1_con == d2_con;
}
void extension(docInfo &d1, docInfo &d2, vector<string> &docs, vector<pair<docInfo, docInfo>> &extend_ret) {
    int d1_id = d1.this_id;
    int d1_lr = d1.this_offset - 1;
    int d1_rl = d1.this_offset + d1.this_length;
    int d2_id = d2.this_id;
    int d2_lr = d2.this_offset - 1;
    int d2_rl = d2.this_offset + d2.this_length;
    string d1_con = docs[d1_id];
    string d2_con = docs[d2_id];
    while (d1_lr >= 0 && d2_lr >= 0 && d1_con[d1_lr] == d2_con[d2_lr]) {
        d1_lr--;
        d2_lr--;
    }
    d1_lr++;
    d2_lr++;
    while (d1_rl < d1_con.size() && d2_rl < d2_con.size() && d1_con[d1_rl] == d2_con[d2_rl]) {
        d1_rl++;
        d2_rl++;
    }
    d1_rl--;
    d2_rl--;
    docInfo ext_d1(d1_id, d1_lr, d1_rl - d1_lr + 1);
    docInfo ext_d2(d2_id, d2_lr, d2_rl - d2_lr + 1);
    extend_ret.push_back(make_pair(ext_d1, ext_d2));
}
bool isInTuple(tuple<int, int, int, int> &t1, tuple<int, int, int, int> &t2) {
    if (get<0>(t1) >= get<0>(t2) && get<1>(t1) <= get<1>(t2) && get<2>(t1) >= get<2>(t2) && get<3>(t1) <= get<3>(t2)) return true;
    return false;
}
bool isInRet(docInfo &di, docInfo &dj, unordered_map<size_t, vector<tuple<int, int, int, int>>> &rets) {
    int si = di.this_offset;
    int ei = di.this_offset + di.this_length - 1;
    int sj = dj.this_offset;
    int ej = dj.this_offset + dj.this_length - 1;
    pair<int, int> doc_pair;
    tuple<int, int, int, int> t_ret;
    if (di.this_id < dj.this_id) {
        doc_pair = make_pair(di.this_id, dj.this_id);
        t_ret = make_tuple(si, ei, sj, ej);
    } else {
        doc_pair = make_pair(dj.this_id, di.this_id);
        t_ret = make_tuple(sj, ej, si, ei);
    }
    size_t index = getPairHash(doc_pair);
    if (rets.find(index) == rets.end()) return false;
    for (auto &item : rets[index]) {
        if (isInTuple(t_ret, item)) return true;
    }
    return false;
}
void updateRets(docInfo &di, docInfo &dj, unordered_map<size_t, vector<tuple<int, int, int, int>>> &rets) {
    int si = di.this_offset;
    int ei = di.this_offset + di.this_length - 1;
    int sj = dj.this_offset;
    int ej = dj.this_offset + dj.this_length - 1;
    pair<int, int> doc_pair;
    tuple<int, int, int, int> t_ret;
    if (di.this_id < dj.this_id) {
        doc_pair = make_pair(di.this_id, dj.this_id);
        t_ret = make_tuple(si, ei, sj, ej);
    } else {
        doc_pair = make_pair(dj.this_id, di.this_id);
        t_ret = make_tuple(sj, ej, si, ei);
    }
    size_t index = getPairHash(doc_pair);
    rets[index].emplace_back(t_ret);
}

// deal with ret.txt file
void refinePairRet(vector<string> &docs, vector<docInfo> &temp_to_refine_ret, vector<pair<docInfo, docInfo>> &final_ret, unordered_map<size_t, vector<tuple<int, int, int, int>>> &rets, float theta = 0.8) {
    for (int i = 0; i < temp_to_refine_ret.size() - 1; i++) {
        docInfo di = temp_to_refine_ret[i];
        for (int j = i + 1; j < temp_to_refine_ret.size(); j++) {
            docInfo dj = temp_to_refine_ret[j];
            s_ext = chrono::high_resolution_clock::now();
            bool flag = isSame(di, dj, docs);
            /*
            bool flag = true;
            if (getJac(di, dj, docs) < theta) flag = false;
            */
            e_ext = chrono::high_resolution_clock::now();
            dur_ext += chrono::duration_cast<chrono::microseconds>(e_ext - s_ext);
            if (!flag) continue;
            /*
            cout << "-----------start-----------"<< endl;
            cout << loadFile(files[di.this_id], di.this_offset, di.this_length) << endl;
            cout << loadFile(files[dj.this_id], dj.this_offset, dj.this_length) << endl;
            cout << "------------end----------"<< endl;
            */
            s_ext = chrono::high_resolution_clock::now();
            if (isInRet(di, dj, rets)) {
                // cout << "already in rets" << endl;
                e_ext = chrono::high_resolution_clock::now();
                dur_ext += chrono::duration_cast<chrono::microseconds>(e_ext - s_ext);
                continue;
            }
            extension(di, dj, docs, final_ret);
            e_ext = chrono::high_resolution_clock::now();
            dur_ext += chrono::duration_cast<chrono::microseconds>(e_ext - s_ext);
            docInfo d1 = final_ret.back().first;
            docInfo d2 = final_ret.back().second;
            s_update = chrono::high_resolution_clock::now();
            updateRets(d1, d2, rets);
            e_update = chrono::high_resolution_clock::now();
            dur_update += chrono::duration_cast<chrono::microseconds>(e_update - s_update);
        }
    }
}

// deduplicate between different clusters
// how to generate the final corpus
void getRet(vector<CompactWindow> &cws, vector<size_t> &keys, vector<vector<docInfo>> &ret) {
    unordered_map<size_t, vector<int>> um;
    for (int i = 0; i < cws.size(); i++) {
        size_t ske = getVectorHash(cws[i].sketch);
        um[ske].emplace_back(i);
    }
    vector<vector<docInfo>> temp_to_refine_ret;
    for (auto &item : um) {
        vector<docInfo> temp;
        if (item.second.size() >= 2) {
            for (auto &index : item.second) {
                temp.emplace_back(cws[index].doc_index, cws[index].beg_range.r, cws[index].end_range.l - cws[index].beg_range.r + 1);
            }
            keys.push_back(item.first);
            ret.push_back(temp);
        }
    }
    // refineRet(files, temp_to_refine_ret, temp_to_extend_ret, intervals);
}
// add edge to similar graph
void addEdgeToSim(pair<int, int> &p_node, vector<g_node> &nodes) {
    int n1 = p_node.first;
    int n2 = p_node.second;
    nodes[n1].sim_neigh.push_back(n2);
    nodes[n2].sim_neigh.push_back(n1);
}

// add edge to overlap graph
void addEdgeToOver(pair<int, int> &p_node, vector<g_node> &nodes) {
    int n1 = p_node.first;
    int n2 = p_node.second;
    nodes[n1].over_neigh.push_back(n2);
    nodes[n2].over_neigh.push_back(n1);
}
void getOverNeiHelper(vector<g_node> &nodes, vector<int> &node_indexes, vector<int> &to_delete) {
    vector<int> temp;
    for (auto &index : node_indexes) {
        for (auto &nei : nodes[index].over_neigh) {
            if (nodes[nei].color == 0) {
                // uncolored
                nodes[nei].color = 1;
                temp.push_back(nei);
                to_delete.push_back(nei);
            }
        }
    }
    if (temp.size() != 0) {
        node_indexes.clear();
        for (auto &item : temp) node_indexes.push_back(item);
        getOverNeiHelper(nodes, node_indexes, to_delete);
    }
}
bool isAllRedNeigh(vector<g_node> &nodes, g_node &node) {
    for (auto &index : node.sim_neigh) {
        if (nodes[index].color != 1) return false;
    }
    return true;
}
void updateNeighToBlack(vector<g_node> &nodes, vector<int> &to_save) {
    // cout << "execute update black" << endl;
    vector<int> temp;
    for (auto &index : to_save) {
        // if (nodes[index].color != 0) cout << "error" << endl;
        nodes[index].color = 2;
        for (auto &index : nodes[index].over_neigh) {
            if (nodes[index].color == 0) {
                // uncolored nodes
                temp.push_back(index);
            }
        }
    }
    if (temp.size() != 0) {
        to_save.clear();
        for (auto &item : temp) to_save.push_back(item);
        updateNeighToBlack(nodes, to_save);
    }
}
void paintBlack(vector<g_node> &nodes, vector<int> &to_delete) {
    for (auto &index : to_delete) {
        for (auto &nei : nodes[index].sim_neigh) {
            if (nodes[nei].color == 0) {
                // without being painted
                if (isAllRedNeigh(nodes, nodes[nei])) {
                    vector<int> to_save;
                    to_save.push_back(nei);
                    updateNeighToBlack(nodes, to_save);
                }
            }
        }
    }
}
void printPaintInfo(vector<g_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        g_node node = nodes[i];
        cout << "current node index is: " << i << endl;
        cout << "its color is: " << node.color << endl;
        cout << "current node sim neighbours are: ";
        for (auto &nei : node.sim_neigh) {
            cout << nei << ", ";
        }
        cout << endl;
        cout << "current node over neighbours are: ";
        for (auto &nei : node.over_neigh) {
            cout << nei << ", ";
        }
        cout << endl;
    }
}
// 0(white): unvisited, 1(red): to delete, 2(black): to save
void Paint(vector<g_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        // 1.find one node without being painted
        if (nodes[i].color != 0) continue;
        cout << "find one node and its index is: " << i << endl;
        nodes[i].color = 1; // paint it with red
        // 2. let all its neighbours and neighbours' neighbours to be red
        vector<int> indexes;
        vector<int> to_delete;
        indexes.push_back(i);
        to_delete.push_back(i);
        getOverNeiHelper(nodes, indexes, to_delete);
        cout << "finish step 2 " << endl;
        // printPaintInfo(nodes);
        // 3.finds all nodes whose neighbours are all red, nodes that should be black
        cout << "to delete content: ";
        for (auto &to_d : to_delete) {
            cout << to_d << ", ";
        }
        cout << endl;
        paintBlack(nodes, to_delete);
        cout << "finish step 3" << endl;
    }
    // printPaintInfo(nodes);
}
bool isInRet(int pos1, int pos2, vector<tuple<int, int, int, int>> &ret, int len) {
    for (auto &item : ret) {
        int s = get<0>(item);
        int l = get<1>(item);
        int s2 = get<2>(item);
        int l2 = get<3>(item);
        if (s <= pos1 && pos1 + len - 1 <= s + l - 1 && s2 <= pos2 && pos2 + len - 1 <= s2 + l2 - 1) return true;
    }
    return false;
}
void buildAllSuffix(const vector<string> &all_text, unordered_map<size_t, vector<pair<int, int>>> &um, int len) {
    hash<string> hasher;
    int index = 0;
    for (auto &text : all_text) {
        for (int i = 0; i < text.size() - len + 1; i++) {
            string substr = text.substr(i, len);
            um[hasher(substr)].emplace_back(index, i);
        }
        index++;
    }
}
void searchInAll(int &doc_index, vector<pair<int, int>> &range, const vector<string> &all_text, unordered_map<size_t, vector<pair<int, int>>> &um, vector<tuple<int, int, int, int>> &ret, int length) {
    hash<string> hasher;
    string text = all_text[doc_index];
    for (int i = 0; i < text.size() - length + 1; i++) {
        string substr = text.substr(i, length);
        size_t hash = hasher(substr);
        for (auto &p : um[hash]) {
            int di = p.first;
            int pos = p.second;
            if (di <= doc_index) continue;
            if (isInRet(i + range[doc_index].first, pos + range[di].first, ret, length)) continue;
            int len = length;
            while (i + len < text.size() && pos + len < all_text[di].size() && text[i + len] == all_text[di][pos + len]) len++;
            tuple<int, int, int, int> t_ret;
            t_ret = make_tuple(i + range[doc_index].first, len, pos + range[di].first, len);
            ret.push_back(t_ret);
        }
    }
}
void searchBySubstr(const string &text, int start, const string &ori_text, int ori_start, suffix_array &sa, vector<tuple<int, int, int, int>> &ret) {
    hash<string> hasher;
    for (int i = 0; i < text.size() - 100 + 1; i++) {
        string substr = text.substr(i, 100);
        size_t hash = hasher(substr);
        if (sa.str_to_index.find(hash) == sa.str_to_index.end()) continue;
        for (auto &pos : sa.str_to_index[hash]) {
            if (isInRet(i + start, pos + ori_start, ret, 100)) continue;
            int len = 100;
            while (i + len < text.size() && pos + len < ori_text.size() && text[i + len] == ori_text[pos + len]) {
                len++;
            }
            tuple<int, int, int, int> t_ret;
            t_ret = make_tuple(start + i, len, pos + ori_start, len);
            ret.push_back(t_ret);
        }
    }
}
void searchSuffix(const string &text, int start, const string &ori_text, int ori_start, char_array &sa, vector<tuple<int, int, int, int>> &ret) {
    for (int i = 0; i < text.size() - 100 + 1; i++) {
        char c = text[i];
        if (sa.char_to_index.find(c) == sa.char_to_index.end()) continue;
        for (auto &pos : sa.char_to_index[c]) {
            if (isInRet(i + start, pos + ori_start, ret, 100)) continue;
            string p1 = text.substr(i, 100);
            string p2 = ori_text.substr(pos, 100);
            if (p1 == p2) {
                int len = 100;
                while (i + len < text.size() && pos + len < ori_text.size() && text[i + len] == ori_text[pos + len]) {
                    len++;
                }
                tuple<int, int, int, int> t_ret;
                t_ret = make_tuple(start + i, len, pos + ori_start, len);
                ret.push_back(t_ret);
            }
        }
    }
}
int32_t getKey(int di, int s, int e) {
    int32_t key = di;
    key *= 7919;
    key += s;
    key *= 7919;
    key += e;
    return key;
}

void getGraph(vector<pair<docInfo, docInfo>> &d_pairs, unordered_map<int, vector<pair<int, int>>> &um_ret, vector<g_node> &nodes) {
    unordered_map<int, vector<pair<int, int>>> um;
    for (auto &pair : d_pairs) {
        int d1 = pair.first.this_id;
        int off1 = pair.first.this_offset;
        int len1 = pair.first.this_length;
        int end1 = off1 + len1 - 1;
        int d2 = pair.second.this_id;
        int off2 = pair.second.this_offset;
        int len2 = pair.second.this_length;
        int end2 = off2 + len2 - 1;
        um[d1].emplace_back(off1, end1);
        um[d2].emplace_back(off2, end2);
    }
    cout << "finish loading pairs info to um" << endl;
    for (auto &item : um) {
        sort(item.second.begin(), item.second.end());
        int start = item.second[0].first;
        int end = item.second[0].second;
        for (int i = 1; i < item.second.size(); i++) {
            int s = item.second[i].first;
            int e = item.second[i].second;
            if (s == start && e == end) {
                continue;
            } else {
                um_ret[item.first].emplace_back(start, end);
                start = s;
                end = e;
            }
        }
        if (um_ret.find(item.first) == um_ret.end() || start != um_ret[item.first].back().first || end != um_ret[item.first].back().second) {
            um_ret[item.first].emplace_back(start, end);
        }
    }
    cout << "finish deduplicating " << endl;
    unordered_map<int32_t, int> di_to_index;
    int index = 0;
    // build index for all nodes here
    for (auto &item : um_ret) {
        cout << "current index is: " << index << endl;
        int di = item.first;
        for (auto &p : item.second) {
            int s = p.first;
            int e = p.second;
            int32_t key = getKey(di, s, e);
            if (di_to_index.find(key) != di_to_index.end()) {
                cout << "error for building index " << endl;
            }
            di_to_index[key] = index;
            index++;
        }
    }
    nodes.clear();
    nodes.resize(index);
    cout << "nodes size is: " << nodes.size() << endl;
    for (auto &item : um_ret) {
        int di = item.first;
        for (int i = 0; i < item.second.size() - 1; i++) {
            int s1 = item.second[i].first;
            int e1 = item.second[i].second;
            for (int j = i + 1; j < item.second.size(); j++) {
                int s2 = item.second[j].first;
                int e2 = item.second[j].second;
                if (s2 <= e1) {
                    // overlap
                    int index_i = di_to_index[getKey(di, s1, e1)];
                    int index_j = di_to_index[getKey(di, s2, e2)];
                    pair<int, int> edge = make_pair(index_i, index_j);
                    addEdgeToOver(edge, nodes);
                }
            }
        }
    }
    for (auto &p : d_pairs) {
        int d1 = p.first.this_id;
        int s1 = p.first.this_offset;
        int len1 = p.first.this_length;
        int e1 = s1 + len1 - 1;
        int d2 = p.second.this_id;
        int s2 = p.second.this_offset;
        int len2 = p.second.this_length;
        int e2 = s2 + len2 - 1;
        int index_i = di_to_index[getKey(d1, s1, e1)];
        int index_j = di_to_index[getKey(d2, s2, e2)];
        pair<int, int> edge = make_pair(index_i, index_j);
        addEdgeToSim(edge, nodes);
    }
}

// print out a vector
template <class T>
void printVec(const vector<T> &vec) {
    for (auto const &tmp : vec) {
        cout << tmp << ",";
    }
    cout << endl;
}