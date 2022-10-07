// C++ program to implement
// external sorting using
// merge sort
#include <bits/stdc++.h>
#include "cw.hpp"
#include "externalSort.hpp"
using namespace std;
 

int main()
{
    // No. of Partitions of input file.
    int num_ways = 10;
 
    // The size of each partition
    int run_size = 1001;
 
    char input_file[]= "input.txt";
    char output_file[]= "output.txt";
    
    ofstream outFile(input_file, ios::out | ios::binary);

    srand(time(NULL));
    
    // generate input
    int tmp;
    for (int i = 0; i < num_ways * 1000; i++)
    {
        tmp = rand();
        outFile.write((char *)&tmp, sizeof(int));
    }
    
    outFile.close();
    
    ExternalSort<int> exSort;
    exSort.externalSort(input_file, output_file, num_ways,
                 run_size);
    
    // printf the sorted result
    ifstream resultFile(output_file,ios::in | ios::binary );
    int pre_int =-1;
    int num = 0;
    bool eof =false;
     while (!eof) {
        eof = (resultFile.peek() ==EOF);
        if(eof == true){
            cout<<"out"<<endl;
            break;
            
        }
        num++;
        int tmp;
        resultFile.read((char*)&tmp, sizeof(int));
        cout<<tmp<<" ";
        assert(tmp >=pre_int);
        pre_int = tmp;

     }
     cout<<endl;
     cout<<num<<endl;
    return 0;
}