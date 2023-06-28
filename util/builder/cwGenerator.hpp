#pragma once
#include <bits/stdc++.h>
#include "../ds/cw.hpp"
#include "../utils.hpp"

const int  MAX_LENGTH = 2000000;

using namespace std;

// the max length of the segment tree (make sure it won't exceed the max length of a document * 4)


/*
    The generator of Compact windows
    Given a document and hash function, it can output the compact windows based on the divide-conquer idea using segment tree
*/
class CwGenerator{
public:
    pair<int,int> hash_func;
    vector<pair<int, int>> seg; // the segment tree
    int INTERVAL_LIMIT;

    CwGenerator(int _t):INTERVAL_LIMIT(_t){
        seg.resize(MAX_LENGTH);
    }

    void set_hf(const pair<int,int> _hf){
        hash_func = _hf;
    }

    // Partition algorithm: In each recurrence, it will search and minimun element in current range and split the range into two pieces
    void partition(const int &doc_id, const vector<int> &doc, int l, int r, vector<vector<CW>> &res_cws) {
        if (l + INTERVAL_LIMIT >= r)
            return;

        pair<int, int> ret(numeric_limits<int>::max(), -1);
        int n = doc.size();
        int a = l, b = r;
        for (a += n, b += n; a <= b; ++a /= 2, --b /= 2) {
            if (a % 2 == 1)
                if (seg[a].first < ret.first)
                    ret = seg[a];
            if (b % 2 == 0)
                if (seg[b].first < ret.first)
                    ret = seg[b];
        }

        // assert(doc[ret.second] >= 0 && doc[ret.second] < tokenNum);
        res_cws[doc[ret.second]].emplace_back(doc_id, l, ret.second, r);
        partition(doc_id, doc, l, ret.second - 1, res_cws);
        partition(doc_id, doc, ret.second + 1, r, res_cws);
    }

    // get the compat windows of one document
    void generate(const int &doc_id, const vector<int> &doc,  vector<vector<CW>> &res_cws) {

        int n = doc.size();
        if (seg.size() < 2 * n) {
            seg.resize(2 * n);
            cout << seg.size() << endl;
        }

        for (int i = 0; i < n; i++) {
            seg[n + i].first = hval(hash_func, doc[i]);
            seg[n + i].second = i;
        }

        for (int i = n - 1; i; i--) {
            if (seg[2 * i].first < seg[2 * i + 1].first)
                seg[i] = seg[2 * i];
            else
                seg[i] = seg[2 * i + 1];
        }

        partition(doc_id, doc, 0, doc.size() - 1, res_cws);
    }

};