#include <iostream>
#include <cmath>
#include <fstream>
using namespace std;

// standard sample rate for wav files
const int sampleRate = 44100;
const int bitDepth = 16;

// The class below creates a simple sinusoidal oscillator

class SineOscillator {
    float frequency, amplitude, angle = 0.0f, offset = 0.0;
public:
    SineOscillator(float freq, float amp) : frequency(freq), amplitude(amp) {
        // offset pertains to the difference to the next sample
        offset = 2 * M_PI * frequency / sampleRate;
    }

    //process holds bulk of the logic for oscillator
    // this needs to be called every sampling interval
    float process() {
        // Asin(2*pi*f/samplingRate)
        auto sample = amplitude * sin(angle);
        angle += offset;
        return sample;
    }
};

void writeToFile(ofstream &file, int value, int size) {
    file.write(reinterpret_cast<const char*> (&value), size);
}

int main() {
    int duration = 2;  //2 seconds

    ofstream audioFile;
    audioFile.open("waveform.wav", ios::binary);  //ios::binary writes as binary

    SineOscillator sineOscillator(440, 0.5);  //new way to set data members

    // Header chunk
    audioFile << "RIFF";  // insertion operator acts like a character array so it's 4 bytes of data. char is 1 byte
    audioFile << "----";  // ---- is place holder, but we
    audioFile << "WAVE";

    // format chunk
    audioFile << "fmt "; //added space to fit 4 bytes
    writeToFile(audioFile, 16, 4); // size
    writeToFile(audioFile, 1, 2); // Compression code
    writeToFile(audioFile, 1, 2); // number of channels
    writeToFile(audioFile, sampleRate, 4);
    writeToFile(audioFile, (sampleRate * bitDepth * 1) / 8, 4 ); // 1 channel, divide by 8 to convert bit to byte rate
    writeToFile(audioFile, bitDepth / 8, 2); // block align
    writeToFile(audioFile, bitDepth, 2); // Bit Depth

    // Data chunk
    audioFile << "data";
    audioFile << "----";

    int preAudioPosition = audioFile.tellp();   //return current position of pointer

    auto maxAmplitude = pow(2, bitDepth - 1) - 1;  //32767 is the highest value 2^16 for cd quality

    for(int i = 0; i < sampleRate * duration; i++) {
        auto sample = sineOscillator.process();
        int intSample = static_cast<int> (sample * maxAmplitude * .01);  // the .01 here adjust the volume
        writeToFile(audioFile, intSample, 2);
    }

    // cout << "Float size: " << sizeof(float) << endl;
    // cout << "Integer size: " << sizeof(int) << endl;

    int postAudioPosition = audioFile.tellp();

    // fill in size in data chunk
    audioFile.seekp(preAudioPosition - 4);
    writeToFile(audioFile, postAudioPosition - preAudioPosition, 4);

    // fill in size in Header chunk
    audioFile.seekp(4, ios::beg);
    writeToFile(audioFile, postAudioPosition - 8, 4);

    audioFile.close();
    return 0;
}
