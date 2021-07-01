
#ifndef _KK_WaveFileReader_h
#define _KK_WaveFileReader_h

#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <cstring>
#include <fstream>
#include <iterator>
#include <algorithm>


//=============================================================
template <class T>
class WaveFileReader
{
public:
    
    //=============================================================
    typedef std::vector<T> AudioBuffer;
    
    //=============================================================
    /** Constructor */
    WaveFileReader();
    
    /** Constructor, using a given file path to load a file */
    WaveFileReader (std::string filePath);
        
    //=============================================================
    /** Loads an audio file from a given file path.
     * @Returns true if the file was successfully loaded
     */
    bool load (std::string filePath);
    
    //=============================================================
    /** @Returns the sample rate */
    uint32_t getSampleRate() const;
    
    /** @Returns the number of audio channels in the buffer */
    int getNumChannels() const;

    /** @Returns true if the audio file is mono */
    bool isMono() const;
    
    /** @Returns true if the audio file is stereo */
    bool isStereo() const;
    
    /** @Returns the bit depth of each sample */
    int getBitDepth() const;
    
    /** Prints a summary of the audio file to the console */
    void printSummary() const;
    
    //=============================================================
    AudioBuffer samples;
    
    //=============================================================
    /** An optional iXML chunk that can be added to the AudioFile. 
     */
    std::string iXMLChunk;
    
private:
    
    //=============================================================
    enum class Endianness
    {
        LittleEndian,
        BigEndian
    };
    
    //=============================================================
    void clearAudioBuffer();
    
    //=============================================================
    int32_t fourBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int16_t twoBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int getIndexOfChunk (std::vector<uint8_t>& source, const std::string& chunkHeaderID, int startIndex, Endianness endianness = Endianness::LittleEndian);
    
    //=============================================================
    T sixteenBitIntToSample (int16_t sample);
    T singleByteToSample (uint8_t sample);
    
    
    //=============================================================
    void reportError (std::string errorMessage);
    
    uint32_t sampleRate;
    uint16_t numChannels;
    int bitDepth;
    bool logErrorsToConsole {true};
};


//=============================================================
enum WavAudioFormat
{
    PCM = 0x0001,
    IEEEFloat = 0x0003,
    ALaw = 0x0006,
    MULaw = 0x0007,
    Extensible = 0xFFFE
};

//=============================================================
/* IMPLEMENTATION */
//=============================================================

//=============================================================
template <class T>
WaveFileReader<T>::WaveFileReader()
{
    bitDepth = 16;
    sampleRate = 44100;
    samples.resize (0);
}

//=============================================================
template <class T>
WaveFileReader<T>::WaveFileReader (std::string filePath)
 :  WaveFileReader<T>()
{
    load (filePath);
}

//=============================================================
template <class T>
uint32_t WaveFileReader<T>::getSampleRate() const
{
    return sampleRate;
}

//=============================================================
template <class T>
int WaveFileReader<T>::getNumChannels() const
{
    return numChannels;
}

//=============================================================
template <class T>
bool WaveFileReader<T>::isMono() const
{
    return getNumChannels() == 1;
}

//=============================================================
template <class T>
bool WaveFileReader<T>::isStereo() const
{
    return getNumChannels() == 2;
}

//=============================================================
template <class T>
int WaveFileReader<T>::getBitDepth() const
{
    return bitDepth;
}

//=============================================================
template <class T>
void WaveFileReader<T>::printSummary() const
{
    std::cout << "|======================================|" << std::endl;
    std::cout << "Num Channels: " << getNumChannels() << std::endl;
    std::cout << "Sample Rate: " << sampleRate << std::endl;
    std::cout << "Bit Depth: " << bitDepth << std::endl;
    std::cout << "|======================================|" << std::endl;
}


//=============================================================
template <class T>
bool WaveFileReader<T>::load (std::string filePath)
{
    std::ifstream file (filePath, std::ios::binary);
    
    // check the file exists
    if (! file.good())
    {
        reportError ("ERROR: File doesn't exist or otherwise can't load file\n"  + filePath);
        return false;
    }
    
    std::vector<uint8_t> fileData;

	file.unsetf (std::ios::skipws);

	file.seekg (0, std::ios::end);
	size_t length = file.tellg();
	file.seekg (0, std::ios::beg);

	// allocate
	fileData.resize (length);

	file.read(reinterpret_cast<char*> (fileData.data()), length);
	
	if (file.gcount() != length)
	{
		reportError ("ERROR: Couldn't read entire file\n" + filePath);
		return false;
	}
    
    // -----------------------------------------------------------
    // HEADER CHUNK
    std::string headerChunkID (fileData.begin(), fileData.begin() + 4);
    //int32_t fileSizeInBytes = fourBytesToInt (fileData, 4) + 8;
    std::string format (fileData.begin() + 8, fileData.begin() + 12);
    
    // -----------------------------------------------------------
    // try and find the start points of key chunks
    int indexOfDataChunk = getIndexOfChunk (fileData, "data", 12);
    int indexOfFormatChunk = getIndexOfChunk (fileData, "fmt ", 12);
    int indexOfXMLChunk = getIndexOfChunk (fileData, "iXML", 12);
    
    // if we can't find the data or format chunks, or the IDs/formats don't seem to be as expected
    // then it is unlikely we'll able to read this file, so abort
    if (indexOfDataChunk == -1 || indexOfFormatChunk == -1 || headerChunkID != "RIFF" || format != "WAVE")
    {
        reportError ("ERROR: this doesn't seem to be a valid .WAV file");
        return false;
    }
    
    // -----------------------------------------------------------
    // FORMAT CHUNK
    int f = indexOfFormatChunk;
    std::string formatChunkID (fileData.begin() + f, fileData.begin() + f + 4);
    //int32_t formatChunkSize = fourBytesToInt (fileData, f + 4);
    uint16_t audioFormat = twoBytesToInt (fileData, f + 8);
    numChannels = twoBytesToInt (fileData, f + 10);
    sampleRate = (uint32_t) fourBytesToInt (fileData, f + 12);
    uint32_t numBytesPerSecond = fourBytesToInt (fileData, f + 16);
    uint16_t numBytesPerBlock = twoBytesToInt (fileData, f + 20);
    bitDepth = (int) twoBytesToInt (fileData, f + 22);
    
    uint16_t numBytesPerSample = static_cast<uint16_t> (bitDepth) / 8;
    
    // check that the audio format is PCM 
    if (audioFormat != WavAudioFormat::PCM)
    {
        reportError ("ERROR: this .WAV file is encoded in a format that this library does not support at present");
        return false;
    }
    
    // check the number of channels is mono or stereo
    if (numChannels < 1 || numChannels > 128)
    {
        reportError ("ERROR: this WAV file seems to be an invalid number of channels (or corrupted?)");
        return false;
    }
    
    // check header data is consistent
    if (numBytesPerSecond != static_cast<uint32_t> ((numChannels * sampleRate * bitDepth) / 8) || numBytesPerBlock != (numChannels * numBytesPerSample))
    {
        reportError ("ERROR: the header data in this WAV file seems to be inconsistent");
        return false;
    }
    
    // check bit depth is either 8, 16, 24 or 32 bit
    if (bitDepth != 16)
    {
        reportError ("ERROR: this file has a bit depth that is not 8, 16, 24 or 32 bits");
        return false;
    }
    
    // -----------------------------------------------------------
    // DATA CHUNK
    int d = indexOfDataChunk;
    std::string dataChunkID (fileData.begin() + d, fileData.begin() + d + 4);
    int32_t dataChunkSize = fourBytesToInt (fileData, d + 4);
    
    int numSamples = dataChunkSize / (bitDepth / 8);

    // numSamples

    int samplesStartIndex = indexOfDataChunk + 8;
    
    clearAudioBuffer();
    samples.resize (numSamples);

    file.seekg (samplesStartIndex, std::ios::beg);
    file.read(reinterpret_cast<char*> (samples.data()), dataChunkSize);

    file.close();

    return true;
}


//=============================================================
template <class T>
void WaveFileReader<T>::clearAudioBuffer()
{   
    samples.clear();
}


//=============================================================
template <class T>
int32_t WaveFileReader<T>::fourBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness)
{
    int32_t result;
    
    if (endianness == Endianness::LittleEndian)
        result = (source[startIndex + 3] << 24) | (source[startIndex + 2] << 16) | (source[startIndex + 1] << 8) | source[startIndex];
    else
        result = (source[startIndex] << 24) | (source[startIndex + 1] << 16) | (source[startIndex + 2] << 8) | source[startIndex + 3];
    
    return result;
}

//=============================================================
template <class T>
int16_t WaveFileReader<T>::twoBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness)
{
    int16_t result;
    
    if (endianness == Endianness::LittleEndian)
        result = (source[startIndex + 1] << 8) | source[startIndex];
    else
        result = (source[startIndex] << 8) | source[startIndex + 1];
    
    return result;
}


//=============================================================
template <class T>
int WaveFileReader<T>::getIndexOfChunk (std::vector<uint8_t>& source, const std::string& chunkHeaderID, int startIndex, Endianness endianness)
{
    constexpr int dataLen = 4;
    if (chunkHeaderID.size() != dataLen)
    {
        assert (false && "Invalid chunk header ID string");
        return -1;
    }

    int i = startIndex;
    while (i < source.size() - dataLen)
    {
        if (memcmp (&source[i], chunkHeaderID.data(), dataLen) == 0)
        {
            return i;
        }

        i += dataLen;
        auto chunkSize = fourBytesToInt (source, i, endianness);
        i += (dataLen + chunkSize);
    }

    return -1;
}

//=============================================================
template <class T>
T WaveFileReader<T>::sixteenBitIntToSample (int16_t sample)
{
    return static_cast<T> (sample) / static_cast<T> (32768.);
}

//=============================================================
template <class T>
T WaveFileReader<T>::singleByteToSample (uint8_t sample)
{
    return static_cast<T> (sample - 128) / static_cast<T> (128.);
}

//=============================================================
template <class T>
void WaveFileReader<T>::reportError (std::string errorMessage)
{
    if (logErrorsToConsole)
        std::cout << errorMessage << std::endl;
}

#endif /* KK_WaveFileReader_h */