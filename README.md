# WavToMP3Encoder

Application to convert wav(16-bitdepth) to mp3. Please provide wavefiles directory path as input. MP3 files are generated in same folder.


# Build command on linux
mkdir build && cd build && cmake .. && make

# Build command on Windows(MSVC compiler)
mkdir build \
cd build \
cmake .. -A Win32 \
msbuild MP3Encoder.sln


# Run application 
for example : \
./MP3Encoder /path/to/wavefiles/directory
