/*
A Segment Tree Class for maintaining the interval maximum and its position.
If there are duplicate maximums, the position is that of the rightest one.
*/
#pragma once
#include <bits/stdc++.h>
using namespace std;
const int MAX_LEN = 50000;

class SegmentTree {
    // Update nodes from bottom to top
    int size;
    int seg_tree[MAX_LEN << 2];
    int max_pos[MAX_LEN << 2];
    int lazy[MAX_LEN << 2];
    int arr[MAX_LEN];
public:
    void clean() {
        memset(arr, 0, sizeof(int)*size);
        memset(lazy, 0, sizeof(lazy));
        memset(seg_tree, 0, sizeof(seg_tree));
        memset(max_pos, 0, sizeof(max_pos));
        size = 0;
    }
    void push_up(int root) {
        if (seg_tree[root << 1] > seg_tree[root << 1 | 1]) {
            seg_tree[root] = seg_tree[root << 1];
            max_pos[root] = max_pos[root << 1];
        } else {
            seg_tree[root] = seg_tree[root << 1 | 1];
            max_pos[root] = max_pos[root << 1 | 1];
        }
    }

    // Update from the top down, left and right children
    void push_down(int root, int L, int R) {
        if (lazy[root]) {
            lazy[root << 1] += lazy[root];
            lazy[root << 1 | 1] += lazy[root];
            seg_tree[root << 1] += lazy[root];
            seg_tree[root << 1 | 1] += lazy[root];
            lazy[root] = 0;
        }
    }

    void build(int root, int L, int R) {
        assert(R < MAX_LEN);
        size = R;
        // lazy[root] = 0;
        if (L == R) {
            seg_tree[root] = arr[L];
            max_pos[root] = L;
            return;
        }
        int mid = (L + R) >> 1;
        build(root << 1, L, mid);
        build(root << 1 | 1, mid + 1, R);
        push_up(root);
    }

    // range query get the maximum in the range [LL,RR]
    pair<int, int> query(int root, int L, int R, int LL, int RR) {
        assert(RR <= size);

        if (LL <= L && R <= RR)
            return make_pair(seg_tree[root], max_pos[root]);
        push_down(root, L, R); // clear lazy tags
        int maximum = 0;
        int max_pos;
        int mid = (L + R) >> 1;
        if (LL <= mid) {
            auto query_pair = query(root << 1, L, mid, LL, RR);
            if (maximum <= query_pair.first) {
                maximum = query_pair.first;
                max_pos = query_pair.second;
            }
        }
        if (RR > mid) {
            auto query_pair = query(root << 1 | 1, mid + 1, R, LL, RR);
            if (maximum <= query_pair.first) {
                maximum = query_pair.first;
                max_pos = query_pair.second;
            }
        }
        return make_pair(maximum, max_pos);
    }

    // range update
    void update_Interval(int root, int L, int R, int LL, int RR, int val) {
        assert(RR <= size);
        assert(LL >= 1);
        assert(root<=size*4);
        // cout << root << endl;
        if (LL <= L && R <= RR) {
            lazy[root] += val;
            seg_tree[root] += val;
            return;
        }
        push_down(root, L, R);
        int mid = (L + R) >> 1;
        if (LL <= mid) update_Interval(root << 1, L, mid, LL, RR, val);
        if (RR > mid) update_Interval(root << 1 | 1, mid + 1, R, LL, RR, val);
        push_up(root);
    }
};