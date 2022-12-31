#include "config.hpp"

using namespace std;

class BuildConfig : public Config {
public:
    string src_file; // the path file of tokenized dataset

    BuildConfig(int _k, int _interval_limit, string _dataset_name, bool _if_attachDocIndex, string _src_file) :
        Config(_k, _interval_limit, _dataset_name, _if_attachDocIndex), src_file(_src_file) {
    }
    
};