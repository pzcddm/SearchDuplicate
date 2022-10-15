// C++ program to implement
// external sorting using
// merge sort
#include <bits/stdc++.h>
#include "util/cw.hpp"
#include "util/new_utils.hpp"
#include "util/externalSort.hpp"
using namespace std;
 

int main()
{
    const unsigned long long total_num = 4220937196;
    // No. of Partitions of input file.
    int num_ways = 100;
 
    // pay attention to if run_size is larger than the maximum of int value !!!
    // The size of each partition
    int run_size = int(total_num/num_ways + 1);
    
    cout<<"run_size: "<< run_size <<endl;
    char input_file[]= "CompatWindowsTrainRaw.bin";
    char output_file[]= "CompatWindowsTrainSorted.bin";
    
    // Timer On
    auto start = LogTime();
    printf("------------------External Sorting Compat Windows------------------\n");

    Wrapped_CW wcw_MAX(INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX);
    ExternalSort<Wrapped_CW> exSort(wcw_MAX);
    exSort.externalSort(input_file, output_file, num_ways,
                 run_size);
    
    printf("------------------Compat Windows Sorted and Written------------------\n");
    // Timer Off
    cout << "Compat Windows Sort Time Cost: " << RepTime(start) << " Seconds\n";

    // Verify the correctness of the algorithm
    // printf the sorted result
    Wrapped_CW tmp;
    Wrapped_CW pre_cw(-1,-1,-1,-1,-1,-1);
    ifstream resultFile(output_file,ios::in | ios::binary );
    unsigned long long num = 0;
    bool eof =false;
     while (!eof) {
        eof = (resultFile.peek() ==EOF);
        if(eof == true){
            cout<<"out"<<endl;
            break;
            
        }
        num++;
        resultFile.read((char*)&tmp, sizeof(Wrapped_CW));
        assert(pre_cw < tmp);
        pre_cw = tmp;

     }
     cout<<endl;
     cout<<num<<endl;
    return 0;
}