#pragma once
#include <bits/stdc++.h>
#include "utils.hpp"

void seq2Int(string sequence, vector<int> &doc, vector<int> &offsets, unordered_map<string, int> &word2id, const unordered_set<string> &stopwords){
    const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";

    vector<string> tokens;
    strToTokens(sequence, delim, tokens, offsets);
    
    for (int i = 0; i < tokens.size(); i++){
        // Skip stop words
        string word = tokens[i];
        if (stopwords.find(word) != stopwords.end())
            continue;
        int wid = word2id[word];
        doc.emplace_back(wid);
    }
}
/*
process original document to a sequence of int values
filename: path to the document @input
doc: store tokens id(int) @output
offsets: offset of each token in the original document @output
word2id: map token to one int value @input
stopWords: all words should be skipped @input
*/
void ProWords2Int(const string &filename, vector<int> &doc, vector<int> &offsets, unordered_map<string, int> &word2id, const unordered_set<string> &stopwords){
    const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";

    // Read from file ...
	ifstream datafile(filename, ios::binary);
    datafile.seekg(0, std::ios_base::end);
    int length = datafile.tellg();
    string docstr(length + 1, '\0');
    datafile.seekg(0);
    datafile.read(&docstr[0], length);

    vector<string> tokens;
    strToTokens(docstr, delim, tokens, offsets);
    
    for (int i = 0; i < tokens.size(); i++){
        // Skip stop words
        string word = tokens[i];
        if (stopwords.find(word) != stopwords.end())
            continue;
        int wid = word2id[word];
        doc.emplace_back(wid);
    }
}
void intToBytes(int paramInt, vector<char> chars)
{
     vector<unsigned char> arrayOfByte(4);
     for (int i = 0; i < 4; i++)
          chars[3 - i] = (paramInt >> (i * 8));
}
//wzz learned sth here
void writeDocByInt(const string &file_name, const string &folder_path, vector<int> &doc){
    string save_file_name = "";
    for (int i = file_name.size() - 1; i >= 0; i--){
        if (file_name[i] == '/'){
            save_file_name = folder_path + file_name.substr(i+1, file_name.size() - 1 - i);
            break;
        }
    }
    assert(save_file_name != "");
    ofstream output_file(save_file_name);
    ostream_iterator<int> output_iterator(output_file, " ");
    copy(doc.begin(), doc.end(), output_iterator);
}
//wzz learned sth here
void loadDocByInt(const string &filename, vector<int> &doc){
    ifstream input_file(filename);
    istream_iterator<int> input_iterator(input_file);
    istream_iterator<int> end{};
    copy(input_iterator, end, std::back_inserter(doc));
}

void PreSketch(const vector<int> &doc, vector<pair<int, int>> &words_hash_pos, vector<Node> &occurrences, vector<Node> &neighbors, vector<int> &word2hash)
{
    for (int word_pos = 0; word_pos < doc.size(); word_pos++)
    {
        words_hash_pos.emplace_back(word2hash[doc[word_pos]], word_pos);
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
    int prev_hv = words_hash_pos.front().first + 1;
    int prev_pos = -1;
    for (auto it = words_hash_pos.begin(); it != words_hash_pos.end(); ++it)
    {
        int hv = it->first;
        int pos = it->second;
        // cout << "pos: " << pos << " hash " << hv << endl;
        if (hv == prev_hv)
        {
            occurrences[pos].prev = prev_pos;
            occurrences[prev_pos].next = pos;
        } 
        else
        {   
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
    
    for (auto rit = words_hash_pos.rbegin(); rit != words_hash_pos.rend(); ++rit)
    {
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
}

//currently used getSkecth interface
//bool value getRet : if it is False, just insert value into the skiplist
void getSketch(int docid, bool getRet, int pos, const vector<int> &doc, vector<Node> &occurrences, vector<Node> &neighbors,  vector<Node> &skiplist, Node &SKIP_TAIL, vector<Node> &D, vector<int> &word2hash, vector<CompactWindow> &cws){
    // first insert into the skip list
    int HEAD_ID = -1;
    int TAIL_ID = doc.size();
    int next = neighbors[pos].next;
    int prev = neighbors[pos].prev;
    int max_hash_value = word2hash[doc[pos]];
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
                int idx = HEAD_REF;
                vector<int> positions;
                vector<int> sketch;
                int rl = 0;
                while (idx != MY_NAN)
                {   
                    //positions.push_back(idx);
                    sketch.push_back(word2hash[doc[idx]]);
                    rl = idx;
                    idx = D[idx].next;
                }
                if (true){
                    if ((rl - cy) >= tau){
                        //res_cws.emplace_back(left_l, cy, rl, right_r);
                        cws.emplace_back(skiplist[cy].prev + 1, cy, rl, cx - 1);
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
