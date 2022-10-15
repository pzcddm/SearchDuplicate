#pragma once
#include <bits/stdc++.h>
#include <dirent.h>

// load the vector<int> of a bin file and push back to docs
void loadBin(const string & binFileName, vector<vector<int>> & docs){
    ifstream ifs(binFileName, ios::binary);
    int size;
    while(ifs.read((char*)&size, sizeof(int))){
        vector<int> vec(size);
        ifs.read((char*)&vec[0], sizeof(int)*size);
        docs.emplace_back(vec);
    }
    ifs.close();
}

// get all the file names in path and put them in a vector
void getFiles(string path, vector<string> &files)
{
    DIR *dr;
    struct dirent *en;
    string file_path;
    dr = opendir(path.c_str()); //open all directory
    if (dr)
    {
        while ((en = readdir(dr)) != NULL)
        {
            //ignore hidden files and folders
            if (en->d_name[0] != '.')
            {
                const char end = path.back();
                if (end == '/')
                    file_path = path + en->d_name;
                else
                    file_path = path + '/' + en->d_name;
                files.push_back(file_path);
            }
        }
        closedir(dr); //close all directory
    }
}

// load all the bin files in a given dir path and push back all their content to docs
void loadDataDir(const string & dir_path, vector<vector<int>> & docs){
    vector<string> binFilesNames;
    getFiles(dir_path, binFilesNames);

    for(auto const & fileName : binFilesNames){
        loadBin(fileName, docs);
    }
}
