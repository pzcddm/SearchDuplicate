#pragma once
#include "cw.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cassert>
// #include <execution>

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

// Line Sweep Algorithm for second dimension
void lineSweep(vector<Point> &points, const int &thres, vector<pair<int, int>> &res_intervals) {
    sort(points.begin(), points.end());
    // sort(std::execution::par_unseq, points.begin(), points.end());
    unordered_set<int> ids;

    int pre_pos = -1;
    int current_pos = points[0].pos;

    for (int i = 0; i < points.size(); i++) {
        // check If iterate to a new position
        if (i > 0 && points[i].pos != points[i - 1].pos)
            // if it meets the minmum intersection amount
            if (ids.size() >= thres) {
                pre_pos = points[i - 1].pos;
                current_pos = points[i].pos;

                res_intervals.push_back(make_pair(pre_pos, current_pos)); // right open interval
            }
        // start point added to set
        // and end point erased from set
        if (points[i].flag == 0) {
            ids.insert(points[i].id);
        } else {
            ids.erase(points[i].id);
        }
    }
}

void lineSweepHelper(const vector<CW> &cw_vet, const unordered_set<int> &ids, vector<pair<int, int>> &intersected_2D_intervals, const int thres) {
    // create the points needed in line sweep algo in second dimension
    vector<Point> tmp_points(ids.size() * 2);
    int tmp_cnt = 0;
    for (auto const &id : ids) {
        tmp_points[tmp_cnt << 1] = Point(cw_vet[id].c, 0, id);
        tmp_points[tmp_cnt << 1 | 1] = Point(cw_vet[id].r + 1, 1, id); // Left closed and right open interval
        tmp_cnt++;
    }
    // Implement Line Sweep Algo in second dimension(those corresponding [c,r])

    lineSweep(tmp_points, thres, intersected_2D_intervals);
}

void nearDupSearch(const vector<CW> &cw_vet, const int thres, vector<CW> &res, unsigned int & winNum) {
    // Get the points from the 1D interval of these compat windows
    vector<Point> points(cw_vet.size() * 2);
    int doc_id = cw_vet[0].T;
    //  cout << "Finding Near Duplicate in this doc_id : "<<doc_id<<endl;
 
    for (int i = 0; i < cw_vet.size(); i++) {
        points[i << 1] = Point(cw_vet[i].l, 0, i);
        points[i << 1 | 1] = Point(cw_vet[i].c + 1, 1, i); // Left closed and right open interval
    }

    sort(points.begin(), points.end());
    //sort(std::execution::par_unseq, points.begin(), points.end());

    // Line Sweep Algorithm
    unordered_set<int> ids;
    int pre_pos = -1;
    int current_pos = points[0].pos;
    for (int i = 0; i < points.size(); i++) {
        // check If iterate to a new position
        if (i > 0 && points[i].pos != points[i - 1].pos) {
            if (ids.size() >= thres) {
                // to get the result of intersected 2D intervals
                vector<pair<int, int>> intersected_2D_intervals;
                lineSweepHelper(cw_vet, ids, intersected_2D_intervals, thres);

                // if get the result
                if (intersected_2D_intervals.size() > 0) {
                    pre_pos = points[i - 1].pos;
                    current_pos = points[i].pos;

                    for (auto const &interval : intersected_2D_intervals) {
                        assert(current_pos <= interval.second);
                        // printf("(%d,%d,%d,%d)\n",pre_pos, current_pos, interval.first, interval.second);
                        winNum += (current_pos - pre_pos) * (interval.second - interval.first);
                        res.emplace_back(doc_id, pre_pos, -1, interval.second - 1); // because of right open interval the r should be minused 1
                    }
                }
            }
        }
        // start point added to set
        // and end point erased from set
        if (points[i].flag == 0) {
            ids.insert(points[i].id);
        } else {
            ids.erase(points[i].id);
        }
    }

    return;
}