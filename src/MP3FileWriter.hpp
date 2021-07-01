#ifndef _KK_MP3FileWriter_h
#define _KK_MP3FileWriter_h

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

class MP3FileWriter
{
    public:

    bool SaveMP3File(std::string file_path, std::vector<uint8_t> mp3_data)
    {
        auto index = 0;
        index = file_path.find(".wav", index);
        if (index == std::string::npos) assert(false);
        /* Make the replacement. */
        file_path.replace(index, 4, ".mp3");

        ofstream myfile { file_path, ofstream::binary };
        
        if (myfile.is_open())
        {
            // myfile << mp3_data.data();
            myfile.write(reinterpret_cast<char const*>(mp3_data.data()),
                    mp3_data.size());
            myfile.close();
            return true;
        }
        else
        {
            cout << "**** MP3FileWriter unable to open" << file_path << std::endl;
            return false;
        }
    }
};

#endif