#ifndef _KK_LameEncoder_h
#define _KK_LameEncoder_h

#include<iostream>
#include<vector>
#include <cassert>
#include"lame.h"

class LameEncoder
{
    public:
    LameEncoder() = delete;
    LameEncoder(uint32_t sample_rate, uint8_t number_of_channels)
    :sample_rate_(sample_rate), number_of_channels_(number_of_channels) 
    {
        gfp = lame_init();

        if (!gfp)
            assert(false);

        if (lame_set_in_samplerate(gfp, sample_rate) != 0)
            assert(false);

        if (lame_set_num_channels(gfp, number_of_channels) != 0)
            assert(false);

        if (lame_set_quality(gfp, 2) != 0)   /* 2=high  5 = medium  7=low */
            assert(false);

        if (lame_init_params(gfp) < 0)
            assert(false);
    }

    ~LameEncoder()
    {
        lame_close(gfp);
        gfp = nullptr; 
    }
    
    std::vector<uint8_t> getMP3EncodedData(std::vector<short int> samples, bool mono)
    {
        std::vector<unsigned char> buffer;

        int numOfSamplePerChannel = samples.size(); 
        if(!mono)
        {
            numOfSamplePerChannel = numOfSamplePerChannel / 2;
        }

        buffer.resize(((numOfSamplePerChannel * 5) / 4) + 7200);
        int encoded_size{};
        if(mono)
        {
            encoded_size = lame_encode_buffer(
            gfp, samples.data(), samples.data(), static_cast<int>(samples.size()),
            buffer.data(), static_cast<int>(buffer.size()));
        }
        else
        {
            encoded_size = lame_encode_buffer_interleaved(
            gfp, samples.data(), static_cast<int>(samples.size() / 2),
            buffer.data(), static_cast<int>(buffer.size()));
        }
        if (encoded_size < 0)
            return {};

        encoded_size +=
            lame_encode_flush(gfp, buffer.data() + encoded_size,
                            static_cast<int>(buffer.size() - encoded_size));

        return { buffer.begin(), buffer.begin() + encoded_size };
            
    }

    private:
    uint32_t sample_rate_;
    uint8_t number_of_channels_;
    lame_global_flags *gfp{nullptr};
};

#endif