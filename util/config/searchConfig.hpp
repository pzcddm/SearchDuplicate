#include "config.hpp"
#include "../utils.hpp"

using namespace std;

class SearchConfig : public Config {
public:
    // file path
    string source_bin_file; // the path of tokenized dataset file
    string tokSeqFile;      // the path of the tokenized query sequence file

    // query config
    int slideWin_len = 64;
    int sample_texts_num;
    int sample_start = 0;       // the query no of texts in the tokSeqFile that
    double prefix_ratio = 0.4; // control prefix length
    double theta = 0.8;         // the minimun similarity threshold
    bool if_in_ilab = false;    // if the current machine is ilab or not
    int max_k = 64;
    int using_k = 64;

    // record result of query
    int total_query_amount = 0;             // the amount of query sequences(sliding windows)
    int found_num = 0;                      // the num of those sequences(sliding windows) have near dup
    int total_np_num = 0;                   // the num of finding np
    double total_query_time = 0;            // toal time overhead of the query
    double total_IO_time = 0;               // IO overhead of the query
    map<double, int> diff_sim_mp;           // record the amount of sequences that meet different similarity threshold
    unordered_map<double, short> sim2index; // mapping three similarity to 0 1 2

    int current_textNo = sample_start;      // current traversed text

private:
    void init_differentSimilarityMp() {
        sim2index.clear();
        diff_sim_mp.clear();

        assert(theta > 0);
        sim2index[theta] = 0;
        sim2index[(1.0 - theta) / 2 + theta] = 1;
        sim2index[1.0] = 2;

        for (auto const pair : sim2index) {
            diff_sim_mp[pair.first] = 0;
        }
    }

public:
    SearchConfig(){}

    SearchConfig(int _max_k, int _interval_limit, string _dataset_name, bool _if_attachDocIndex, string _tokSeqFile) :
        Config(_max_k, _interval_limit, _dataset_name, _if_attachDocIndex), tokSeqFile(_tokSeqFile) {
        max_k = _max_k;
        using_k = max_k;
        if_in_ilab = checkHostnameContainsIlab();
    }

    void setQueryConfig(int _slideWin_len, int _sample_texts_num, int _sample_st, double _prefix_ratio, double _theta){
        slideWin_len = _slideWin_len;
        sample_texts_num = _sample_texts_num;
        sample_start = _sample_st;
        prefix_ratio = _prefix_ratio;
        theta = _theta;
        current_textNo = sample_start;
    }

    void setTheta(double _theta) {
        theta = _theta;
        init_differentSimilarityMp();
    }

    void setUsingK(int _k) {
        using_k = _k;
    }

    void loadTokenziedSeqs(vector<vector<int>> &tokenizedSeqs) {
        loadBin(tokSeqFile, tokenizedSeqs);
    }

    void parseArgv(const int &argc, char **&argv) {
        for (int i = 0; i < argc; i++) {
            const string arg = argv[i];
            if (arg == "-dataset") {
                string _dataset_name = string(argv[i + 1]);
                if (_dataset_name == "pile")
                    config_dataset(_dataset_name, 210607728);
                else if (_dataset_name == "openwebtext")
                    config_dataset(_dataset_name, 8013769);
                else if (_dataset_name == "c4")
                    config_dataset(_dataset_name, 364868892);
                else
                    perror("No given dataset\n");
            }

            if (arg == "-slideWin_len") {
                slideWin_len = atoi(argv[i + 1]);
            }

            if (arg == "-tokSeqFile") {
                tokSeqFile = string(argv[i + 1]);
            }

            if (arg == "-wordNum") {
                set_tokenNums(atoi(argv[i + 1]));
            }

            if (arg == "-docNum") {
                set_docLimit(atoi(argv[i + 1]));
            }

            if (arg == "-T") {
                set_t(atoi(argv[i + 1]));
            }

            if (arg == "-sample_texts_num") {
                sample_texts_num = atoi(argv[i + 1]);
            }

            if (arg == "-k") {
                k = atoi(argv[i + 1]);
                max_k = k;
                using_k = k;
            }

            if (arg == "-prefix_ratio") {
                prefix_ratio = stod(string(argv[i + 1]));
            }

            if (arg == "-theta") {
                theta = atof(argv[i + 1]);
            }

            if (arg == "-sample_start") {
                sample_start = atoi(argv[i + 1]);
                current_textNo = sample_start;
            }
        }
        init_differentSimilarityMp();
    }

    // exact different similarity from compact windows and increase corresponding similarity record in the diff_sim_mp
    void exactDiffSim(const vector<CW> &duplicateCWs){
        if(duplicateCWs.size() == 0)
            return;
        
        // if the duplicateCWs is not empty, that means there must be a compact window that meets the minimum similarity threshold
        bool flags[3] = {false, false, false};
        flags[0] = true;

        // iterate duplicateCWs and analyze if meeting different similarity
        for(auto const & cw : duplicateCWs){
            for(auto const &pair:sim2index){
                if(cw.c * 1.0 / using_k >= pair.first){
                    flags[pair.second] = true;
                }
            }

            if(flags[2] == true){
                break;
            }
        }

        // store the result into diff_sim_mp
        for(auto const &pair:sim2index){
            diff_sim_mp[pair.first] += flags[pair.second];
        }
    }

    void display_parameters() {
        printf("---------Displaying The Parameters Of SearchConfig-----------\n");
        printf("dataset: %s queryFile:%s \n", dataset_name.c_str(), tokSeqFile.c_str());
        printf("tokenNum: %d ,max_k: %d using_k: %d, T:%d , zonemap_interval: %d, zoneMpSize: %d\n", token_num, max_k, using_k, interval_limit, zonemp_interval, zoneMpSize);
    }

    void display_curInfo(){
        cout << "current sequences no: " << current_textNo << endl;
        cout << "current traversed sequences number: " << current_textNo - sample_start + 1 << endl;
        cout << "current traversed windows: " << total_query_amount << endl;
    }

    void display_diffSimMp(){
        for (auto const &it : diff_sim_mp) {
            printf("similarity:%f passage_num:%d\n", it.first, it.second);
        }
    }

    void show_result() {
        printf("---------Showing The Result Of This Seach-----------\n");
        printf("queryFile:%s \n",tokSeqFile.c_str());
        printf("Have traversed %d windows of %d texts in %s from No.%dnd text to No.%dnd text\n", total_query_amount, sample_texts_num, tokSeqFile.c_str(), sample_start, current_textNo);
        printf("near duplicate windows found: %d\n", found_num);
        display_diffSimMp();
        printf("memorized squences amount: %d  total_np_num: %d\n average query cost: %f average IO cost: %f, average caculation cost: %f", found_num, total_np_num, total_query_time / total_query_amount, total_IO_time / total_query_amount, (total_query_time - total_IO_time) / total_query_amount);
    }
};