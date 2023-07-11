#include "util/config/buildConfig.hpp"
#include "util/builder/builder.hpp"
using namespace std;

/*
    This code is for building the index of compact windows
    Attention: It only is used for dataset that size is lower than your memory
*/
int main(int argc, char **argv) {
    string index_dir = "./index";
    string dataset_name = "openwebtext";  // openwebtext c4
    const int k = 64;                 // the number of hash functions
    const int t = 50;                 // set the interval limit for generating compat windows
    const int zonemp_interval = 5000; // the stride that decreasing when generating zonemap
    const int zoneMpSize = 50257;     // the size of zonemaps under one hashfunction

    BuildConfig build_config(k, t, dataset_name, false);
    build_config.parseArgv(argc,argv);
    build_config.createFileDirPath(index_dir); // build the building index directory under current parameters
    build_config.display_parameters();         // display the parameters

    // Start building index
    IndexBuilder index_builder(build_config);
    index_builder.build();  
}