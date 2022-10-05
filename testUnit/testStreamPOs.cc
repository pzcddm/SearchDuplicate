#include <iostream>
#include <fstream>
using namespace std;
int main(){
    static_assert(sizeof(std::streamoff) <= sizeof(long long), "Oops.");
    cout<<sizeof(std::streamoff)<<endl;
}