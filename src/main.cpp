#include<iostream>
#include<thread>

#ifdef _WIN32
#include <windows.h>
#define HAVE_STRUCT_TIMESPEC 1
#else
#include <unistd.h>
#endif

#include"ExtractWaveFile.hpp"
#include"LameEncoder.hpp"
#include"WaveFileReader.hpp"
#include"MP3FileWriter.hpp"

using namespace std;

unsigned hardware_concurrency()
{
#ifdef _WIN32
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwNumberOfProcessors;
#else
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void EncodeToMP3(std::string filePath)
{
    cout << "************************************\n";
    std::cout << filePath << std::endl; 
    WaveFileReader<short int> audioFile; 

    if(audioFile.load(filePath))
    {
        audioFile.printSummary();

        LameEncoder mp3_encoder(audioFile.getSampleRate(), audioFile.getNumChannels());

        std::vector<uint8_t> mp3_encoded_data; 

        if(audioFile.isMono())
        {   
            mp3_encoded_data = mp3_encoder.getMP3EncodedData(audioFile.samples, true);
            MP3FileWriter mp3_file_writer;
            mp3_file_writer.SaveMP3File(filePath, mp3_encoded_data);
        }
        else if(audioFile.isStereo())
        {
            mp3_encoded_data = mp3_encoder.getMP3EncodedData(audioFile.samples, false);
            MP3FileWriter mp3_file_writer;
            mp3_file_writer.SaveMP3File(filePath, mp3_encoded_data);
        } 
        else
        {
            assert(false);
        }
    }
}

void EncodeWaveListToMP3(std::vector<std::string> wave_file_list, int startIndex, int endIndex)
{
    for(int i = startIndex; i < endIndex; ++i)
    {
        EncodeToMP3(wave_file_list[i]);
    }
}

int main(int argc, char *argv[])
{
    cout << "Hello World\n";

    ExtractWaveFile exctract_wave_file;

    const auto & wave_file_list = exctract_wave_file.getWaveFiles(argv[1]);

    std::vector<std::thread > thread_pool;
    size_t available_cpu = hardware_concurrency();
    auto thread_pool_size = (available_cpu < wave_file_list.size()) ? available_cpu : wave_file_list.size();
    thread_pool.reserve(thread_pool_size);

    if (thread_pool_size == wave_file_list.size())
    {
        for(auto i : wave_file_list)
        {
            thread_pool.emplace_back(std::thread(EncodeToMP3, i));
        }
        for(int i = 0; i < thread_pool.size(); ++i)
        {
            thread_pool[i].join();
        }
    }
    else
    {
        auto wave_files_per_thread = wave_file_list.size() / thread_pool_size;
        auto wave_files_for_last_thread = wave_files_per_thread + (wave_file_list.size() % thread_pool_size);

        auto startIndex = 0;
        auto endIndex = 0;
        for(int i = 0; i < thread_pool_size; ++i)
        {
            if(i == (thread_pool_size-1))
            {
                endIndex += wave_files_for_last_thread;
            }
            else
            {
                endIndex += wave_files_per_thread;
            }
            thread_pool.emplace_back(std::thread(EncodeWaveListToMP3, wave_file_list, startIndex, endIndex));
            startIndex = endIndex;
        }
        for(int i = 0; i < thread_pool_size; ++i)
        {
            thread_pool[i].join();
        }
    }
    
    return 0;
}