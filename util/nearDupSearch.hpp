#pragma once
#include "cw.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cassert>

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

void lineSweep(vector<Point> &points, const int &thres, vector<pair<int, int>> &res_intervals) {
    sort(points.begin(), points.end());

    // Line Sweep Algorithm
    unordered_set<int> ids;
    int intersection_st = -1;
    int intersection_ed = -1;
    for (int i = 0; i < points.size(); i++) {
        // start point added to set
        // and end point erased from set
        if (points[i].flag == 0) {
            // the situation that thet size of set will be upon thres soon
            if (intersection_st == -1 && ids.size() == thres - 1) {
                intersection_st = points[i].pos;
            }
            ids.insert(points[i].id);
        } else {
            // if current point is a end point, that means in this position, there won't be any start points
            // because the sort operation makes start points first in one position
            if (intersection_st != -1 && ids.size() == thres) {
                intersection_ed = points[i].pos;

                cout << "intersection_st_2d: " << intersection_st << " ed: " << intersection_ed << endl;
                res_intervals.push_back(make_pair(intersection_st, intersection_ed));

                // clear intersection_st
                intersection_st = -1;
                intersection_ed = -1;
            }
            ids.erase(points[i].id);
        }

        // // if this point it the last point, end this set and record the status
        if (i == points.size() - 1 && intersection_st != -1) {
            assert(ids.size() >= thres);
            intersection_ed = points[i].pos;

            cout << "intersection_st_2d: " << intersection_st << " ed: " << intersection_ed << endl;
            res_intervals.push_back(make_pair(intersection_st, intersection_ed));
        }
    }
}

void lineSweepHelper(const vector<CW> &cw_vet, const int &doc_id, const unordered_set<int> &ids, const int &intersection_st, const int thres, vector<CW> &res) {
    // create the points needed in line sweep algo in second dimension
    vector<Point> tmp_points(ids.size() * 2);
    int tmp_cnt = 0;
    for (auto const &id : ids) {
        tmp_points[tmp_cnt << 1] = Point(cw_vet[id].c, 0, id);
        tmp_points[tmp_cnt << 1 | 1] = Point(cw_vet[id].r, 1, id);
        tmp_cnt++;
    }
    // Implement Line Sweep Algo in second dimension(those corresponding [c,r])
    vector<pair<int, int>> intersected_2D_intervals;
    lineSweep(tmp_points, thres, intersected_2D_intervals);

    // Simply choose the farthest r and push it into the result
    if (intersected_2D_intervals.size() > 0) {
        int farthest_r = -1;
        for (auto const &pa : intersected_2D_intervals) {
            farthest_r = max(farthest_r, pa.second);
        }
        res.emplace_back(doc_id, intersection_st, -1, farthest_r);
    }
}

void nearDupSearch(const vector<CW> &cw_vet, const int thres, vector<CW> &res) {
    // Get the points from the 1D interval of these compat windows
    vector<Point> points(cw_vet.size() * 2);
    int doc_id = cw_vet[0].T;
    cout << "Finding Near Duplicate in this doc_id : "<<doc_id<<endl;
    
    for (int i = 0; i < cw_vet.size(); i++) {
        points[i << 1] = Point(cw_vet[i].l, 0, i);
        points[i << 1 | 1] = Point(cw_vet[i].c, 1, i);
    }

    sort(points.begin(), points.end());

    // Line Sweep Algorithm
    unordered_set<int> ids;
    int intersection_st = -1;
    int intersection_ed = -1;
    for (int i = 0; i < points.size(); i++) {
        // start point added to set
        // and end point erased from set
        if (points[i].flag == 0) {
            // the situation that thet size of set will be upon thres soon
            if (intersection_st == -1 && ids.size() == thres - 1) {
                intersection_st = points[i].pos;
            }
            ids.insert(points[i].id);
        } else {
            // if current point is a end point, that means in this position, there won't be any start points
            // because the sort operation makes start points first in one position

            // the situation that the size of set will be lower than thres soon
            // cout << "i: " << i <<endl;
            // cout <<" id: " << points[i].id <<endl;
            // cout <<"ids size: "<< ids.size()<<endl;
            if (intersection_st != -1 && ids.size() == thres) {
                intersection_ed = points[i].pos;

                cout << "intersection_st: " << intersection_st << " ed: " << intersection_ed << endl;
                // Use LineSweepAlgo to find intersected range in [c,r]s
                lineSweepHelper(cw_vet, doc_id, ids, intersection_st, thres, res);

                // clear intersection_st
                intersection_st = -1;
                intersection_ed = -1;
            }
            ids.erase(points[i].id);
        }

        // // if this point it the last point, end this set and record the status
        if (i == points.size() - 1 && intersection_st != -1) {
            assert(ids.size() >= thres);
            intersection_ed = points[i].pos;

            cout << "intersection_st: " << intersection_st << " ed: " << intersection_ed << endl;
            // Implement line Sweep Algo in second dimension(those corresponding [c,r])
            lineSweepHelper(cw_vet, doc_id, ids, intersection_st, thres, res);
        }
    }

    return;
}