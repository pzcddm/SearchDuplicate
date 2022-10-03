#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <cassert>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <chrono>
#include <cstring>

using namespace std;
int test_var = 250;

bool comp(vector<int> &a, vector<int> &b){
    assert(a.size() == b.size());
    for (int i = a.size() - 1; i >= 0; i--){
        if (a[i] > b[i]){
            return true;
        }else if (a[i] < b[i]){
            return false;
        }
    }
    return false;
}
class g_node{
public:
    int doc_index;
    int color; //0(white): unvisited, 1(red): to delete, 2(black): to save
    vector<int> sim_neigh;
    vector<int> over_neigh;
    g_node() {
        doc_index = 0;
        color = 0;
    }
};
class c_subset
{
public:
    int doc_index;
    vector<int> ske;
    c_subset(){}
};

class k_sketch
{
public:
    int cur_max_hv;
    int doc_index;
    vector<int> ske;
    k_sketch() {}
    k_sketch(int max_hv, int index, vector<int> &sketch){
        cur_max_hv = max_hv;
        doc_index = index;
        for (auto &item: sketch){
            ske.push_back(item);
        }
    }
};
class Range
{
public:
    int l;
    int r;
    Range(int a, int b)
        : l(a), r(b) {}
    Range()
    {
        l = 0;
        r = 0;
    }
};
class plaInfo
{
public:
    int this_id;
    int this_offset;
    int this_length;
    int that_id;
    int that_offset;
    int that_length;
    plaInfo() {}
    plaInfo(int d1, int d2, Range &beg1, Range &end1, Range &beg2, Range &end2){
        this_id = d1;
        this_id = d2;
        this_offset = beg1.l;
        this_length = end1.r - beg1.l;
        that_offset = beg2.l;
        that_length = end2.r - beg2.l;
    }
};

class docInfo
{
public:
    int this_id;
    int this_offset;
    int this_length;
    docInfo() {}
    docInfo(int id, Range &beg, Range &end){
        this_id = id;
        this_offset = beg.l;
        this_length = end.r - beg.l;
    }
    docInfo(int id, int offset, int length){
        this_id = id;
        this_offset = offset;
        this_length = length;
    }
};

class nodeInfo
{
public:
    int docid;
    int max_hv;
    nodeInfo() {}
    nodeInfo(int doc_index, int hv){
        docid = doc_index;
        max_hv = hv;
    }
};
bool compareNode(nodeInfo &n1, nodeInfo &n2){
    if (n1.max_hv > n2.max_hv) {
        return true;
    }else if(n1.max_hv < n2.max_hv){
        return false;
    }else return n1.docid > n2.docid; 
}
class CompactWindow
{
public:
    Range beg_range;
    Range end_range;
    int max_hv; // sub_sketch: contain all items with the same mod value 
    int doc_index = 0;
    vector<int> sketch; // stores k hash values
    CompactWindow() {}
    CompactWindow(int ll, int lr, int rl, int rr) {
        beg_range.l = ll;
        beg_range.r = lr;
        end_range.l = rl;
        end_range.r = rr;
    }
    CompactWindow(Range& beg, Range &end, int hv, int docid, vector<int> ske){
        beg_range.l = beg.l;
        beg_range.r = beg.r;
        end_range.l = end.l;
        end_range.r = end.r;
        max_hv = hv;
        doc_index = docid;
        for (auto &item: ske){
            sketch.push_back(item);
        }
    }
};

bool compCW(CompactWindow &c1, CompactWindow &c2){
    for (int i = c1.sketch.size() - 1; i >= 0; i--){
        if (c1.sketch[i] > c2.sketch[i]){
            return true;
        }else if (c1.sketch[i] < c2.sketch[i]){
            return false;
        }
    }
    return false;
}
//just compare whether they have the same sketch
bool isSameCW(CompactWindow &c1, CompactWindow &c2){
    for (int i = 0; i < c1.sketch.size(); i++){
        if (c1.sketch[i] != c2.sketch[i]){
            return false;
        }
    }
    return true;
}
bool geCW(CompactWindow &c1, CompactWindow &c2){
    for (int i = c1.sketch.size() - 1; i >= 0; i--){
        if (c1.sketch[i] > c2.sketch[i]){
            return true;
        }else if (c1.sketch[i] < c2.sketch[i]){
            return false;
        }
    }
    return true;
}
class HeapSke
{
public:
    vector<CompactWindow> cws;
    HeapSke() {}
    HeapSke(vector<CompactWindow> &com_wins, int docid){
        for (auto &cw: com_wins){
            cws.push_back(cw);
            cws.back().doc_index = docid;
        }
        make_heap(cws.begin(), cws.end(), compCW);
    }
    void pop_back_min(CompactWindow &min_ske){
        pop_heap(cws.begin(), cws.end(), compCW);
        min_ske = cws.back();
        cws.pop_back();
    }
    void pop_min_comp(CompactWindow &cur_min, CompactWindow &min_ske){
        while (cws.size() != 0){
            pop_back_min(min_ske);
            if (geCW(min_ske, cur_min)) break;
        }
    }
};

class CompositeWindow
{
public:
    vector<int> sketch; // stores k hash values
    vector<int> positions; //stores k positions to the hash values
    int sub_index; // sub_sketch: contain all items with the same mod value 
    Range beg_range;
    Range end_range;
    CompositeWindow() {}
    CompositeWindow(int ll, int lr, int rl, int rr) {
        beg_range.l = ll;
        beg_range.r = lr;
        end_range.l = rl;
        end_range.r = rr;
    }
};

class Node
{
public:
    int prev;
    int next;
    Node(int p, int n)
        : prev(p), next(n) {}
    Node()
    {
        prev = -999;
        next = -999;
    }
};

class windowPair 
{
public:
  int docOffset;
  int docLen;
  int queryOffset;
  int queryLen;
  windowPair(int docOffset, int docLen, int queryOffset, int queryLen) {
    this->docOffset = docOffset;
    this->docLen = docLen;
    this->queryOffset = queryOffset;
    this->queryLen = queryLen;
  }
};

class RangeWindow
{
public:
    int hash_value;
    int position;
    vector<pair<Range, Range>> occur_ranges;
    RangeWindow(int hv, int pos){
        hash_value = hv;
        position = pos;
    } 
};

class suffix_array
{
public:
    unordered_map<size_t, vector<int>> str_to_index;
    suffix_array(){
        str_to_index.clear();
    }
    void build(const string &text){
        hash<string> hasher;
        for (int i = 0; i < text.size() - 99; i++){
            string substr = text.substr(i, 100);
            str_to_index[hasher(substr)].emplace_back(i);
        }
    }
};

class char_array
{
public:
    unordered_map<char, vector<int>> char_to_index;
    char_array(){
        char_to_index.clear();
    }
    void build(const string &text){
        for (int i = 0; i < text.size() - 99; i++){
            char c = text[i];
            char_to_index[c].emplace_back(i);
        }
    }
};