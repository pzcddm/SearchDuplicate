#pragma once
#include "config.hpp"

using namespace std;

class BuildConfig : public Config {
public:
    string src_file; // the path file of tokenized dataset
    BuildConfig(){}
    BuildConfig(int _k, int _interval_limit, string _dataset_name, bool _if_attachDocIndex) :
        Config(_k, _interval_limit, _dataset_name, _if_attachDocIndex) {
            if (_dataset_name == "pile")
                src_file = "/research/projects/zp128/dataset_tokenizedGbt2/tokenized_bin/pile_gpt2.bin";
            else if (_dataset_name == "openwebtext")
                src_file = "/research/projects/zp128/dataset_tokenizedGbt2/tokenized_bin/openwebtext_gpt2.bin";
            else if (_dataset_name == "c4")
                src_file = "/research/projects/zp128/dataset_tokenizedGbt2/tokenized_bin/c4_en_train_gpt2.bin";
    }

     void parseArgv(const int &argc, char **&argv) {
        for (int i = 0; i < argc; i++) {
            const string arg = argv[i];

            if (arg == "-doc_limit") {
                set_docLimit(atoi(argv[i + 1]));
            }

            if (arg == "-t") {
                set_t(atoi(argv[i + 1]));
            }

            if (arg == "-k") {
                k = atoi(argv[i + 1]);
            }
        }
    }

};