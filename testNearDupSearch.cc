#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "util/nearDupSearch.hpp"
#include "util/cw.hpp"

using namespace std;

int main(){
    vector<CW> cw_vet;
    vector<CW> res_cw;
    cw_vet.emplace_back(0, 0,2,4);
    cw_vet.emplace_back(0, 1,3,5);
    cw_vet.emplace_back(0, 1,4,5);

    nearDupSearch(cw_vet, 2, res_cw);

    cout<< "the intersected range is "<<endl;
    for(auto const & cw : res_cw){
        cout<<"left boundary: "<<cw.l<<" right boundary: "<<cw.r<<endl;
    }
}