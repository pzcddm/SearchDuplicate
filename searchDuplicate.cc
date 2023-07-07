#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "util/ds/cw.hpp"
#include "util/ds/docIndex.hpp"
#include "util/ds/bigIndexItem.hpp"

#include "util/IO.hpp"
#include "util/utils.hpp"
#include "util/new_utils.hpp"
#include "util/stopwordsFilter.hpp"
#include "util/config/searchConfig.hpp"
#include "util/query.hpp"
// #include "util/queryFaster.hpp"
#include "util/dupSearch/segmentTree.hpp"
using namespace std;

// global variables
DocIndex **docIndexArr;
BigIndexItem **indexArr;
// intiliaze thread_num segment tree for parelled near duplicate search
vector<SegmentTree> trees(omp_get_max_threads());
ZoneMaps zonemaps;
vector<pair<int, int>> hashFunctions;

int wordNum;
int docNum;

void prepareGlobalVariables(int k) {
    cout << "total token amount: " << wordNum << endl;
    // the hash functions' seeds are 1 to k (cannot use 0 and 1 both together because their hash functions are the same)
    for (int i = 1; i <= k; i++) generateHashFunc(i, hashFunctions);
}

int reportPassagesNum(const vector<CW> &duplicateCWs) {
    int pasNum = 0;
    CW tmp_cw(0, -1, -1, -1);
    for (auto const &cw : duplicateCWs) {
        if (tmp_cw.intersected(cw)) {
            tmp_cw.merge(cw);
        } else {
            pasNum++;
            tmp_cw = cw;
        }
    }
    return pasNum;
}

void delete_220_token(vector<int> &vec) {
    for (auto it = vec.begin(); it != vec.end();) {
        if ((*it) == 220)
            it = vec.erase(it);
        else
            ++it;
    }
}

int main(int argc, char **argv) {
    // Default parameters for config
    string source_bin_file = "../dataset_tokenizedGbt2/openwebtext_gpt2.bin";
    string dataset = "c4";                                                    // string dataset = "pile";
    int max_k = 64;                                                           // the maximum number of hash functions
    int T = 50;                                                               // the T used in generating compact windows
    // string tokSeqFile = "../SelfGenerationText/gpt-neo-540L_50TOPK_1_3B.bin"; // "../SelfGenerationText/gpt2-medium-540L_50TOPK_400000S.bin"  "./pile_sampled_docs.bin"
    // string tokSeqFile = "sampled_dataset/openwebtext_samples.bin";

    string tokSeqFile = "../ChatGptInvestigation/WebScraping_ShareGpt/tokenized_answers/1st_english_answers.bin";
    string parent_dir = "./index";
    string stopwords_bin_path = "./filtered_tokens.bin";
    bool if_attachDocIndex = false;

    // Default parameters for searching
    int slideWin_len = 64; // or 128 or 32
    int sample_sequence_num = 5000;
    int sample_start = 0;
    bool if_showPassage = false;
    double prefix_ratio = 0.4; // control prefix length
    float theta = 0.8;         // similarity threshold

    SearchConfig config(max_k, T, dataset, if_attachDocIndex, tokSeqFile);
    config.setQueryConfig(slideWin_len, sample_sequence_num, sample_start, prefix_ratio, theta);

    // parse the arguments
    config.parseArgv(argc, argv);

    wordNum = config.token_num;
    docNum = config.doc_limit; // the amount of texts in the dataset 210607728 8013769

    // load document index
    // string doc_index_file = "./doc_index/openwebtext_gpt2_docIndex.bin";
    vector<unsigned long long> doc_index;
    // readDocInex(doc_index, doc_index_file);

    // show current parameters of config
    config.display_parameters();

    // get the data path
    config.getFileDirPath(parent_dir);

    // load the tokenized sequences
    vector<vector<int>> tokenizedSeqs;
    config.loadTokenziedSeqs(tokenizedSeqs);

    prepareGlobalVariables(config.max_k);

    // Initialize and load zone map
    zonemaps = ZoneMaps(config.max_k, config.zoneMpSize);
    zonemaps.load(config.zoneMap_dirPath);

    // load the IndexItem
    loadBigIndexItem(indexArr, wordNum, config.max_k, config.index_filePath);

    // load the docIndex
    if (if_attachDocIndex) {
        loadDocIndex(docIndexArr, wordNum, config.max_k, config.docIndex_filePath);
    }

    map<int, int> mp;
    StopwordsFilter token_filter(stopwords_bin_path);

    for (int i = 0; i < config.sample_texts_num; i++) {

        if(i+sample_start >= tokenizedSeqs.size())  break;

        auto &raw_seq = tokenizedSeqs[i + config.sample_start];
        delete_220_token(raw_seq);
        // make sure the sequence length is long enough
        if (raw_seq.size() < config.slideWin_len) {
            cout << "Meet short seq, skip " << endl;
            continue;
        }

        cout << "New Sequence length: " << raw_seq.size() << endl;
        config.display_curInfo();
        config.current_textNo++;
        if(config.if_in_ilab == true)
            config.if_in_ilab = system("keep-job 48");

        for (int j = 0; j + slideWin_len <= raw_seq.size(); j += slideWin_len) {
            config.total_query_amount++;
            vector<int> seq;
            seq.assign(raw_seq.begin() + j, raw_seq.begin() + slideWin_len + j);
            token_filter.filter_erase(seq); // filter stopwords
            double query_time;
            unsigned int cwNum = 0;
            Query query(seq, config.theta, config.using_k, config.cw_dirPath, config.prefix_ratio);
            // QueryFaster query(seq, theta, k, cw_dir, prefilter_size, t_dir_path, docOfs_dir_path);
            // Search near duplicate sentence
            vector<CW> duplicateCWs = query.getResult(cwNum, query_time);
            config.total_IO_time += query.getIOtime();
            config.exactDiffSim(duplicateCWs);
            config.total_query_time += query_time;

            // output the duplicate passage
            if (if_showPassage && duplicateCWs.size()) {
                int max_id;
                int max_length = 0;
                for (int m = 0; m < duplicateCWs.size(); m++) {
                    auto tmp_length = duplicateCWs[m].r - duplicateCWs[m].l;
                    if (tmp_length >= max_length) {
                        max_id = m;
                        max_length = tmp_length;
                    }
                }
                cout << "Print one of the duplicate passage:\n";
                vector<int> dup_passage;
                auto cw = duplicateCWs[max_id];
                getPassage(cw.T, doc_index, source_bin_file, cw.l, cw.r, dup_passage);
                printVec(dup_passage);
            }

            // Extract differtent theta from different
            int np_passagesNum = reportPassagesNum(duplicateCWs);
            if (np_passagesNum > 0) {
                config.found_num++;

                if (if_showPassage) {
                    cout << "Show the query seq:\n";
                    printVec(seq);
                }

                mp[np_passagesNum]++;
                config.total_np_num += np_passagesNum;
                config.display_diffSimMp();
            }
        }
    }

    for (auto const &it : mp) {
        printf("np: %d value: %d\n", it.first, it.second);
    }

    config.show_result();
}
