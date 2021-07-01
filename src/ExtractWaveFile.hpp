#ifndef _KK_ExtractWaveFile_h
#define _KK_ExtractWaveFile_h

#include <fstream>
#include<iostream>
#include<string>
#include<vector>
#include<filesystem>

namespace fs = std::filesystem;

class ExtractWaveFile
{

    std::vector<std::string> list_of_wave_files_{};
public:

    std::vector<std::string> getWaveFiles(std::string path)
    {
        std::string ext{".wav"};

        for(const auto & file : fs::directory_iterator(path))
        {
            if(file.path().extension() == ext)
            {
                list_of_wave_files_.push_back(file.path().string());
            }
        }
        return list_of_wave_files_;
    }

};

#endif