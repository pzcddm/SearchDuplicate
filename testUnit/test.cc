#include<bits/stdc++.h>

using namespace std;

int main(){
    ifstream ifs("test.bin", ios::binary);
    int size;
    while(ifs.read((char*)&size, sizeof(int))){
        vector<int> vec(size);
        ifs.read((char*)& (vec[0]), sizeof(int)*size);
        for(int i =0 ;i<size;i++){
            cout<<vec[i]<<endl;
        }
    }

}