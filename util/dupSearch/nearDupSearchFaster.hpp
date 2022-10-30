#pragma once
#include <bits/stdc++.h>

#include "../cw.hpp"
#include "segmentTree.hpp"
#include "discretization.hpp"

using namespace std;

struct Point {
    int pos;   // position of the start point or end point
    bool flag; // start point(false) or end point(1)
    int id;    // the id of corresponding interval

    Point() {
    }

    Point(int _pos, bool _flag, int _id) :
        pos(_pos), flag(_flag), id(_id) {
    }

    bool operator<(const Point &tmp) const {
        if (pos == tmp.pos) {
            return flag < tmp.flag;
        }
        return pos < tmp.pos;
    }
};

void nearDupSearchFaster(const vector<CW> &cw_vet, const int thres, vector<CW> &res, SegmentTree& segTree) {
    // Get the points from the 1D interval of these compat windows
    vector<Point> points(cw_vet.size() * 2);
    int doc_id = cw_vet[0].T;
    //  cout << "Finding Near Duplicate in this doc_id : "<<doc_id<<endl;
    
    // Discretization Tool
    Discret discret;

    //Initilize Points Array and push each c and r into Discret
    for (int i = 0; i < cw_vet.size(); i++) {
        points[i << 1] = Point(cw_vet[i].l, 0, i);
        points[i << 1 | 1] = Point(cw_vet[i].c + 1, 1, i); // Left closed and right open interval

        // push c and r into Discret
        discret.push_arr(cw_vet[i].c);
        discret.push_arr(cw_vet[i].r);
    }

    sort(points.begin(), points.end());
    //sort(std::execution::par_unseq, points.begin(), points.end());

    // execute diecretization
    discret.exe();
    int discret_size = discret.getArrSize();
    
    // build segTree for judging whether there is intersection in second dimension
    segTree.build(1,1,discret_size);
    
    // Line Sweep Algorithm
    unordered_set<int> ids;
    int pre_pos = -1;
    int current_pos = points[0].pos;
    for (int i = 0; i < points.size(); i++) {
        // check If iterate to a new position
        if (i > 0 && points[i].pos != points[i - 1].pos) {
            if (ids.size() >= thres) {

                // find the maximum in this segTree
                // auto query_pair = segTree.query(1, 1, discret_size, points[i].pos-1, discret_size);
                auto query_pair = segTree.query(1, 1, discret_size, 1, discret_size);
                int rev_discret_pos = discret.rev_discret(query_pair.second);
                // cout<<"rev_discret_pos:"<< rev_discret_pos<<" max value "<<query_pair.first<<endl;
                // if get the result
                if (query_pair.first >= thres) {
                    pre_pos = points[i - 1].pos;
                    current_pos = points[i].pos;

                    cout<< "currentpos and rev_discret"<<current_pos<<" "<<rev_discret_pos<<endl;
                    assert(current_pos-1 <= rev_discret_pos);
                    res.emplace_back(doc_id, pre_pos, -1, rev_discret_pos); // because of right open interval the r should be minused 1
                    // printf("(%d,%d,%d,%d)\n",pre_pos, current_pos, interval.first, interval.second);
                }
            }
        }

        auto corres_cw = cw_vet[points[i].id];
        int discreted_c = discret.discrete(corres_cw.c);
        int discreted_r = discret.discrete(corres_cw.r);
        assert(discreted_c<=discreted_r);
        
        // start point added to set
        // and end point erased from set
        // update segement tree
        if (points[i].flag == 0) {
            ids.insert(points[i].id);
            segTree.update_Interval(1,1,discret_size,discreted_c,discreted_r,1);
        } else {
            ids.erase(points[i].id);
            segTree.update_Interval(1,1,discret_size,discreted_c,discreted_r,-1);
        }
    }
    segTree.clean();
    return;
}