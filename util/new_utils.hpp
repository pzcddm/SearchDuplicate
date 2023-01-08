#pragma once

#include "utils.hpp"
#include <bits/stdc++.h>
#include <dirent.h>

template<class T> 
vector<unsigned> sort_index(vector<T> arr){
    vector<pair<T,unsigned>> pair_arr(arr.size());
    for(unsigned i = 0;i<pair_arr.size();i++){
        pair_arr[i]=make_pair(arr[i],i);
    }
    sort(pair_arr.begin(),pair_arr.end());

    vector<unsigned> res(arr.size());
    for(unsigned i = 0;i<pair_arr.size();i++){
        res[i]=pair_arr[i].second;
    }
    return res;
}

void seq2Int(string sequence, vector<int> &doc, vector<int> &offsets, unordered_map<string, int> &word2id, const unordered_set<string> &stopwords){
    const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";

    vector<string> tokens;
    strToTokens(sequence, delim, tokens, offsets);
    
    for (int i = 0; i < tokens.size(); i++){
        // Skip stop words
        string word = tokens[i];
        if (stopwords.find(word) != stopwords.end())
            continue;
        int wid = word2id[word];
        doc.emplace_back(wid);
    }
}

/*
process original document to a sequence of int values
filename: path to the document @input
doc: store tokens id(int) @output
offsets: offset of each token in the original document @output
word2id: map token to one int value @input
stopWords: all words should be skipped @input
*/
void ProWords2Int(const string &filename, vector<int> &doc, vector<int> &offsets, unordered_map<string, int> &word2id, const unordered_set<string> &stopwords){
    const string delim = "\t\n\r\x0b\x0c !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~,.!?;";

    // Read from file ...
	ifstream datafile(filename, ios::binary);
    datafile.seekg(0, std::ios_base::end);
    int length = datafile.tellg();
    string docstr(length + 1, '\0');
    datafile.seekg(0);
    datafile.read(&docstr[0], length);

    vector<string> tokens;
    vector<int> token_offsets;
    strToTokens(docstr, delim, tokens, token_offsets);
    
    for (int i = 0; i < tokens.size(); i++){
        // Skip stop words
        string word = tokens[i];
        if (stopwords.find(word) != stopwords.end())
            continue;
        if (word2id.find(word) == word2id.end()){
            cout << "error" << endl;
        }
        int wid = word2id[word];
        doc.emplace_back(wid);
        offsets.emplace_back(token_offsets[i]);
    }
}
void int2bytes(vector<int> nums, vector<char> &chars)
{
    for (auto &nint: nums){
        char byte1 =  nint & 0x000000ff;
        char byte2 = (nint & 0x0000ff00) >> 8;
        char byte3 = (nint & 0x00ff0000) >> 16;
        char byte4 = (nint & 0xff000000) >> 24;
        cout << "the size of one byte is: " << sizeof(byte1) << endl;
        chars.push_back(byte1);
        chars.push_back(byte2);
        chars.push_back(byte3);
        chars.push_back(byte4);
    }
}
//wzz learned sth here
void writeDocByInt(const string &file_name, const string &folder_path, vector<int> &doc){
    string save_file_name = "";
    for (int i = file_name.size() - 1; i >= 0; i--){
        if (file_name[i] == '/'){
            save_file_name = folder_path + file_name.substr(i+1, file_name.size() - 1 - i);
            break;
        }
    }
    assert(save_file_name != "");
    ofstream output_file(save_file_name);
    ostream_iterator<int> output_iterator(output_file, " ");
    copy(doc.begin(), doc.end(), output_iterator);
}
void bytes2intPointer(char* chars, long chars_size, vector<int> &nums){
    for (int i = 0; i < chars_size; i+=4){
        int a = int((unsigned char)(chars[i]) |
            (unsigned char)(chars[i+1]) << 8 |
            (unsigned char)(chars[i+2]) << 16 |
            (unsigned char)(chars[i+3]) << 24 );
        nums.push_back(a);
    }
}
void bytes2int(vector<char> &chars, long chars_size, vector<int> &nums){
    for (int i = 0; i < chars_size; i+=4){
        int a = int((unsigned char)(chars[i]) |
            (unsigned char)(chars[i+1]) << 8 |
            (unsigned char)(chars[i+2]) << 16 |
            (unsigned char)(chars[i+3]) << 24 );
        nums.push_back(a);
    }
}
void writeDocByBytes(const string &file_name, const string &folder_path, vector<int> &doc, vector<int> &offsets, vector<int> &split, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    ofstream of_byte(byte_file_name, ios_base::binary);
    of_byte.write((char*)&doc[0], doc.size() * sizeof(int));
    ofstream of_off(offset_file_name, ios_base::binary);
    of_off.write((char*)&offsets[0], offsets.size() * sizeof(int));
    ofstream of_split(split_file_name, ios_base::binary);
    of_split.write((char*)&split[0], split.size() * sizeof(int));
    ofstream of_fs(files_file_name);
    ostream_iterator<string> of_fs_iter(of_fs, "\n");
    copy(files.begin(), files.end(), of_fs_iter);
}
void loadBytesToInt(const string &file_name, vector<int> &vec){
    cout << "current file name is: " << file_name << endl;
    ifstream ifs(file_name, ios::binary);
    ifs.seekg(0, std::ios_base::end);
    long length = ifs.tellg();
    cout << "its length is: " << length << endl;
    char* chars = new char [length];
    ifs.seekg(0);
    ifs.read(&chars[0], length);
    bytes2intPointer(chars, length, vec);
    cout << "finish loading this file info" << endl;
}
void loadBytesToIntOneByOne(const string &file_name, vector<int> &split, vector<vector<int>> &vecs){
    ifstream ifs(file_name, ios::binary);
    for (int i = 0; i < split.size() - 1; i++){
        vector<int> vec;
        int start = split[i];
        int end = split[i+1];
        int length = (end - start) * 4;
        vector<char> chars(length);
        ifs.seekg(start * 4);
        ifs.read(&chars[0], length);
        bytes2int(chars, length, vec);
        vecs.push_back(vec);
    }
}
void loadDocByBytesOnebyOne(const string &file_name, const string &folder_path, vector<vector<int>> &docs, vector<vector<int>> &offsets, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    vector<int> split;
    loadBytesToInt(split_file_name, split);
    loadBytesToIntOneByOne(byte_file_name, split, docs);
    cout << "finish loading docs info" << endl;
    loadBytesToIntOneByOne(offset_file_name, split ,offsets);
    cout << "finish loading offsets info" << endl;
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
}

void loadDocByBytes(const string &file_name, const string &folder_path, vector<vector<int>> &docs, vector<vector<int>> &offsets, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string offset_file_name = byte_file_name + ".offsets";
    string split_file_name = byte_file_name + ".split";
    string files_file_name = byte_file_name + ".files";
    vector<int> split;
    loadBytesToInt(split_file_name, split);
    cout << "split size is: " << split.size() << endl;
    vector<int> total_doc;
    loadBytesToInt(byte_file_name, total_doc);
    cout << "total token number is: " << total_doc.size() << endl;
    vector<int> total_offset;
    loadBytesToInt(offset_file_name, total_offset);
    cout << "total offset number is: " << total_offset.size() << endl;
    for (int i = 0; i < split.size() - 1; i++){
        int start = split[i];
        int end = split[i+1];
        vector<int> sub_doc_vec = {total_doc.begin() + start, total_doc.begin() + end};
        docs.push_back(sub_doc_vec);
        vector<int> sub_off_vec = {total_offset.begin() + start, total_offset.begin() + end};
        offsets.push_back(sub_off_vec);
    }
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
}

// only load the documents names into files vector
void loadFilesNameByBytes(const string &file_name, const string &folder_path, vector<string> &files){
    string byte_file_name = folder_path + file_name;
    string files_file_name = byte_file_name + ".files";
    ifstream input_file(files_file_name);
    istream_iterator<string> input_iterator(input_file);
    istream_iterator<string> end{};
    copy(input_iterator, end, std::back_inserter(files));
}

//wzz learned sth here
void loadDocByInt(const string &filename, vector<int> &doc){
    ifstream input_file(filename);
    istream_iterator<int> input_iterator(input_file);
    istream_iterator<int> end{};
    copy(input_iterator, end, std::back_inserter(doc));
}

// cited from https://itecnote.com/tecnote/c-file-count-in-a-directory-using-c/
// count the file numbers
int getDirFileNum(string& dirPath)
{
  DIR *dp;
  int i = 0;
  struct dirent *ep;     
  dp = opendir (dirPath.c_str());

  if (dp != NULL)
  {
    while (ep = readdir (dp))
      i++;

    (void) closedir (dp);
  }
  else
    perror ("Couldn't open the directory");

  return i;
}