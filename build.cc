#include "util/config/buildConfig.hpp"
#include "util/builder/builder.hpp"
using namespace std;

int main(){
    string index_dir = "./index";
    string dataset_name = "openwebtext";
    const int k = 64;                        // the number of hash functions
    const int t = 50;               // set the interval limit for generating compat windows
    const int zonemp_interval = 5000;  // the stride that decreasing when generating zonemap
    const int zoneMpSize = 50257;       // the size of zonemaps under one hashfunction

    BuildConfig build_config(k, t, dataset_name, false);
    build_config.createFileDirPath(index_dir);       // build the building index directory under current parameters
    build_config.display_parameters();  // display the parameters

    // Start building index
    IndexBuilder index_builder(build_config);
    index_builder.build();
}