#pragma once

#include "utils.hpp"
#include <bits/stdc++.h>

template<class T> 
vector<unsigned> sort_index(vector<T> arr){
    vector<pair<T,unsigned>> pair_arr(arr.size());
    for(unsigned i = 0;i<pair_arr.size();i++){
        pair_arr[i]=make_pair(arr[i],i);
    }
    sort(pair_arr.begin(),pair_arr.end());

    vector<unsigned> res(arr.size());
    for(unsigned i = 0;i<pair_arr.size();i++){
        res[i]=pair_arr[i].second;
    }
    return res;
}

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
    vector<int> token_offsets;
    strToTokens(docstr, delim, tokens, token_offsets);
    
    for (int i = 0; i < tokens.size(); i++){
        // Skip stop words
        string word = tokens[i];
        if (stopwords.find(word) != stopwords.end())
            continue;
        if (word2id.find(word) == word2id.end()){
            cout << "error" << endl;
        }
        int wid = word2id[word];
        doc.emplace_back(wid);
        offsets.emplace_back(token_offsets[i]);
    }
}
void int2bytes(vector<int> nums, vector<char> &chars)
{
    for (auto &nint: nums){
        char byte1 =  nint & 0x000000ff;
        char byte2 = (nint & 0x0000ff00) >> 8;
        char byte3 = (nint & 0x00ff0000) >> 16;
        char byte4 = (nint & 0xff000000) >> 24;
        cout << "the size of one byte is: " << sizeof(byte1) << endl;
        chars.push_back(byte1);
        chars.push_back(byte2);
        chars.push_back(byte3);
        chars.push_back(byte4);
    }
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
void bytes2intPointer(char* chars, long chars_size, vector<int> &nums){
    for (int i = 0; i < chars_size; i+=4){
        int a = int((unsigned char)(chars[i]) |
            (unsigned char)(chars[i+1]) << 8 |
            (unsigned char)(chars[i+2]) << 16 |
            (unsigned char)(chars[i+3]) << 24 );
        nums.push_back(a);
    }
}
void bytes2int(vector<char> &chars, long chars_size, vector<int> &nums){
    for (int i = 0; i < chars_size; i+=4){
        int a = int((unsigned char)(chars[i]) |
            (unsigned char)(chars[i+1]) << 8 |
            (unsigned char)(chars[i+2]) << 16 |
            (unsigned char)(chars[i+3]) << 24 );
        nums.push_back(a);
    }
}
void writeDocByBytes(const string &file_name, const string &folder_path, vector<int> &doc, vector<int> &offsets, vector<int> &split, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    ofstream of_byte(byte_file_name, ios_base::binary);
    of_byte.write((char*)&doc[0], doc.size() * sizeof(int));
    ofstream of_off(offset_file_name, ios_base::binary);
    of_off.write((char*)&offsets[0], offsets.size() * sizeof(int));
    ofstream of_split(split_file_name, ios_base::binary);
    of_split.write((char*)&split[0], split.size() * sizeof(int));
    ofstream of_fs(files_file_name);
    ostream_iterator<string> of_fs_iter(of_fs, "\n");
    copy(files.begin(), files.end(), of_fs_iter);
}
void loadBytesToInt(const string &file_name, vector<int> &vec){
    cout << "current file name is: " << file_name << endl;
    ifstream ifs(file_name, ios::binary);
    ifs.seekg(0, std::ios_base::end);
    long length = ifs.tellg();
    cout << "its length is: " << length << endl;
    char* chars = new char [length];
    ifs.seekg(0);
    ifs.read(&chars[0], length);
    bytes2intPointer(chars, length, vec);
    cout << "finish loading this file info" << endl;
}
void loadBytesToIntOneByOne(const string &file_name, vector<int> &split, vector<vector<int>> &vecs){
    ifstream ifs(file_name, ios::binary);
    for (int i = 0; i < split.size() - 1; i++){
        vector<int> vec;
        int start = split[i];
        int end = split[i+1];
        int length = (end - start) * 4;
        vector<char> chars(length);
        ifs.seekg(start * 4);
        ifs.read(&chars[0], length);
        bytes2int(chars, length, vec);
        vecs.push_back(vec);
    }
}
void loadDocByBytesOnebyOne(const string &file_name, const string &folder_path, vector<vector<int>> &docs, vector<vector<int>> &offsets, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    vector<int> split;
    loadBytesToInt(split_file_name, split);
    loadBytesToIntOneByOne(byte_file_name, split, docs);
    cout << "finish loading docs info" << endl;
    loadBytesToIntOneByOne(offset_file_name, split ,offsets);
    cout << "finish loading offsets info" << endl;
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
}

void loadDocByBytes(const string &file_name, const string &folder_path, vector<vector<int>> &docs, vector<vector<int>> &offsets, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    vector<int> split;
    loadBytesToInt(split_file_name, split);
    cout << "split size is: " << split.size() << endl;
    vector<int> total_doc;
    loadBytesToInt(byte_file_name, total_doc);
    cout << "total token number is: " << total_doc.size() << endl;
    vector<int> total_offset;
    loadBytesToInt(offset_file_name, total_offset);
    cout << "total offset number is: " << total_offset.size() << endl;
    for (int i = 0; i < split.size() - 1; i++){
        int start = split[i];
        int end = split[i+1];
        vector<int> sub_doc_vec = {total_doc.begin() + start, total_doc.begin() + end};
        docs.push_back(sub_doc_vec);
        vector<int> sub_off_vec = {total_offset.begin() + start, total_offset.begin() + end};
        offsets.push_back(sub_off_vec);
    }
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
}

// only load the documents names into files vector
void loadFilesNameByBytes(const string &file_name, const string &folder_path, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string files_file_name = byte_file_name + ".files";
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
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
void getSketch(vector<int> offset, int docid, bool getRet, int pos, const vector<int> &doc, vector<Node> &occurrences, vector<Node> &neighbors,  vector<Node> &skiplist, Node &SKIP_TAIL, vector<Node> &D, vector<int> &word2hash, vector<CompactWindow> &cws){
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

//without offset info
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
int find_set(int* parents, int x)  
{
  while (parents[x] != x)  
  {
    parents[x] = parents[parents[x]];
    x = parents[x];
  }
  return x;
}

void union_set(int* rank, int* parents, int x, int y)
{
  // Replace nodes by roots
  x = find_set(parents, x);
  y = find_set(parents, y);

  // x and y are already in the same set
  if (x == y) return;

  // If necessary, rename variables to ensure that
  // x has rank at least as large as that of y
  if (rank[x] < rank[y]) 
    swap(x, y); 

  // Make x the new root
  parents[y] = x;
  // If necessary, increment the rank of x
  if (rank[x] == rank[y])
    rank[x] = rank[x] + 1;
}
//extend in original docs
void extendCluRetByInt(vector<vector<int>> &docs, vector<docInfo> &to_extend_ret, unordered_map<int, vector<int>> &di2id, vector<vector<docInfo>> &clu_ret){
    vector<int> doc_ids;
    vector<int> start_pos;
    vector<int> end_pos;
    for (auto &item: to_extend_ret){
        doc_ids.push_back(item.this_id);
        start_pos.push_back(item.this_offset - 1);
        end_pos.push_back(item.this_offset + item.this_length);
    }
    while(1){
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++){
            int si = start_pos[i];
            int sni = start_pos[i+1];
            if (si < 0 || sni < 0 || docs[i][si] != docs[i+1][sni]){
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++){
            //if (doc_ids.size() > 2) cout << "really did extension for cluster here " << endl;
            start_pos[i] = start_pos[i] - 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++){
        start_pos[i] = start_pos[i] + 1;
    }
    while(1){
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++){
            int ei = end_pos[i];
            int eni = end_pos[i+1];
            if (ei >= docs[i].size() || eni >= docs[i+1].size() || docs[i][ei] != docs[i+1][eni]){
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++){
            end_pos[i] = end_pos[i] + 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++){
        end_pos[i] = end_pos[i] - 1;
    }
    vector<docInfo> extend_ret;
    for (int i = 0; i < doc_ids.size(); i++){
        extend_ret.emplace_back(doc_ids[i], start_pos[i], end_pos[i] - start_pos[i] + 1);
        di2id[doc_ids[i]].emplace_back(clu_ret.size());
    }
    clu_ret.push_back(extend_ret);
}

//extend in original docs
void extendCluRetByStr(vector<string> &docs, vector<docInfo> &to_extend_ret, unordered_map<int, vector<int>> &di2id, vector<vector<docInfo>> &clu_ret){
    vector<int> doc_ids;
    vector<int> start_pos;
    vector<int> end_pos;
    for (auto &item: to_extend_ret){
        doc_ids.push_back(item.this_id);
        start_pos.push_back(item.this_offset - 1);
        end_pos.push_back(item.this_offset + item.this_length);
    }
    while(1){
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++){
            int si = start_pos[i];
            int sni = start_pos[i+1];
            if (si < 0 || sni < 0 || docs[i][si] != docs[i+1][sni]){
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++){
            //if (doc_ids.size() > 2) cout << "really did extension for cluster here " << endl;
            start_pos[i] = start_pos[i] - 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++){
        start_pos[i] = start_pos[i] + 1;
    }
    while(1){
        bool flag = false;
        for (int i = 0; i < doc_ids.size() - 1; i++){
            int ei = end_pos[i];
            int eni = end_pos[i+1];
            if (ei >= docs[i].size() || eni >= docs[i+1].size() || docs[i][ei] != docs[i+1][eni]){
                flag = true;
                break;
            }
        }
        if (flag) break;
        for (int i = 0; i < doc_ids.size(); i++){
            end_pos[i] = end_pos[i] + 1;
        }
    }
    for (int i = 0; i < doc_ids.size(); i++){
        end_pos[i] = end_pos[i] - 1;
    }
    vector<docInfo> extend_ret;
    for (int i = 0; i < doc_ids.size(); i++){
        extend_ret.emplace_back(doc_ids[i], start_pos[i], end_pos[i] - start_pos[i] + 1);
        di2id[doc_ids[i]].emplace_back(clu_ret.size());
    }
    clu_ret.push_back(extend_ret);
}
/*
check whether dis is one subset of currently found results(clu_ret) 
whether dis is one found ret
*/
bool isSubRet(vector<docInfo> &dis, unordered_map<int, vector<int>> &di2id, vector<vector<docInfo>> &clu_ret){
    unordered_set<int> clu_ids; // record cluster id to its frequency
    unordered_map<int, int> di2index; // docInfo to its index
    int did = dis[0].this_id;
    di2index[did] = 0;
    if ( di2id.find(did) == di2id.end()) return false;
    for (auto &clu_id: di2id[did]){
        clu_ids.insert(clu_id);
    }
    for (int i = 1; i < dis.size(); i++){
        unordered_set<int> temp_set;
        did = dis[i].this_id;
        di2index[did] = i;
        if ( di2id.find(did) == di2id.end()) return false;
        for (auto &clu_id: di2id[did]){
            if (clu_ids.find(clu_id) != clu_ids.end()){
                temp_set.insert(clu_id);
            }
        }
        if (temp_set.size() == 0) return false;
        clu_ids = temp_set;
    }
    //check the clu ids
    for (auto &clu_id: clu_ids){
        for (auto &di: clu_ret[clu_id]){
            if (di2index.find(di.this_id) == di2index.end()) continue;
            int index = di2index[di.this_id];
            int off1 = dis[index].this_offset;
            int len1 = dis[index].this_length;
            int off2 = di.this_offset;
            int len2 = di.this_length;
            if (off2 > off1 || off2 + len2 < off1 + len1) return false;
        }
    }
    return true;
}
void Cluster(vector<vector<int>> &docs, vector<docInfo> &vec_di, vector<vector<docInfo>> &clu_ret, int &total_clu, unordered_map<int, vector<int>> &di2id, float theta = 0.8){
    int* parents;
    int* rank;
    int N = vec_di.size();
    parents = new int[N];
    rank = new int[N];
    for (int i = 0; i < N; i++) {
        parents[i] = i;
        rank[i] = 0;
    }
    for (int i = 0; i < vec_di.size() - 1; i++){
        docInfo di = vec_di[i];
        for (int j = i+1; j < vec_di.size(); j++){
            docInfo dj = vec_di[j];
            if (getJacByInt(di, dj, docs) >= theta){
                union_set(rank, parents, i, j);
            }
        }
    }
    unordered_map<int, vector<int>> clu;
    for (int i = 0; i < N; i++){
        int p = find_set(parents, i);
        clu[p].emplace_back(i);
    }
    bool flag = true;
    for (auto &item: clu){
        //cout << "in the same item: " << endl;
        if (item.second.size() >= 2){
            if (flag){
                total_clu++;
                flag = false;
            }
            //cout << "index info in this clu: " << endl;
            vector<docInfo> temp_ret;
            for (auto &index: item.second){
                temp_ret.push_back(vec_di[index]);
            }
            if (isSubRet(temp_ret, di2id, clu_ret)) continue;
            extendCluRetByInt(docs, temp_ret, di2id, clu_ret);
        }
    }
}
void printOutRet(vector<vector<int>> &docs, vector<vector<docInfo>> &clu_ret){
    for (auto &dis: clu_ret){
        cout << "------------------------------------" << endl;
        cout << "current near-duplicate passages in this cluster: " << endl;
        for (auto &di: dis){
            int did = di.this_id;
            int off = di.this_offset;
            int len = di.this_length;
            cout << "content here: ------------------ docid is: " << did << endl;
            for (int i = 0; i < len; i++){
                cout << docs[did][i+off] << ", ";
            }
            cout << "--------------------------------" << endl;
        }
        cout << "------------------------------------" << endl << endl;
    }
}
void getPartialSketchByContent(string &content, unordered_map<string, int> &word2id, const unordered_set<string> &stopWords, vector<int> &word2hash){
    const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";
    vector<string> tokens;
    vector<int> tokensOffsets;
    vector<int> words;
    vector<string> save_words;
    cout << "original doc info is: " << content << endl;
    strToTokens(content, delim, tokens, tokensOffsets);
    cout << "final tokens are: " << endl;
    for (int i = 0; i < tokens.size(); i++){
        if (stopWords.find(tokens[i]) == stopWords.end()){
            cout << tokens[i] << ", ";
            int wid = word2id[tokens[i]];
            words.emplace_back(wid); 
        }
    }
    cout << endl << "the wid for each token is: "  << endl;
    for (auto &wid: words){
        cout << wid << ", ";
    }
    cout << endl << endl;
    vector<int> sketch;
    cout << "original sketch is: " << endl;
    for (auto &wid: words){
        sketch.push_back(word2hash[wid]);
        cout << word2hash[wid] << ", ";
    }
    cout << "number of sketch is: " << sketch.size() << endl;
    sort(sketch.begin(), sketch.end());
    cout << "the sketch is: " << endl;
    for (int i = 0; i < sketch.size(); i++){
        cout << sketch[i] << ", ";
    }
}
/*
//interface for debug
void getJac2Str(const string &con1, const string &con2){

}
*/
bool isOverlap(docInfo &di, docInfo &dj, float theta = 0.8){
    // no overlap
    int off1 = di.this_offset;
    int end1 = off1 + di.this_length;
    int off2 = dj.this_offset;
    int end2 = off2 + dj.this_length;
    //no overlap
    if (end1 <= off2 || end2 <= off1) return false;
    float overlap = (min(end1, end2) - max(off1, off2)) / (max(end2, end1) - min(off1, off2));
    if (overlap >= theta) return true;
    return false;
}
//interface for merging cluster whose passage share long overlap

void mergeClu(vector<vector<docInfo>> &clu_ret, vector<vector<docInfo>> &merge_ret){
    //vector<docInfo> all_dis; // store all doc infos
    // get each di info
    unordered_map<int, vector<pair<int, int>>> di2index;
    for (int i = 0; i < clu_ret.size(); i++){
        //i is the group index
        for (int j = 0; j < clu_ret[i].size(); j++){
            di2index[clu_ret[i][j].this_id].emplace_back(i,j);
        }
    }
    // initialize union set
    int* parents;
    int* rank;
    int N = clu_ret.size();
    parents = new int[N];
    rank = new int[N];
    for (int i = 0; i < N; i++) {
        parents[i] = i;
        rank[i] = 0;
    }
    // find overlap over threshold
    for (auto &item: di2index){
        for (int i = 0; i < item.second.size() - 1; i++){
            pair<int, int> p_i = item.second[i];
            docInfo di = clu_ret[p_i.first][p_i.second];
            for (int j = i+1; j < item.second.size(); j++){
                pair<int, int> p_j = item.second[j];
                docInfo dj = clu_ret[p_j.first][p_j.second];
                if (p_i.first == p_j.first) continue;
                //use dong's code merge union
                if (isOverlap(di, dj)){
                    union_set(rank, parents, i, j);
                }
            }
        }
    }
    unordered_map<int, vector<int>> clu;
    for (int i = 0; i < N; i++){
        int p = find_set(parents, i);
        clu[p].emplace_back(i);
    }
    for (auto &item: clu){
        vector<docInfo> temp_ret;
        for (auto &index: item.second){
            temp_ret.insert(temp_ret.end(), clu_ret[index].begin(), clu_ret[index].end());
        }
        merge_ret.push_back(temp_ret);
    }
}

