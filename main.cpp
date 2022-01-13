#include "app/Init.hpp"
#include "app/LogHandler.hpp"


#include "libs/fft/FFTReal.h"
#include "libs/fft/FFTRealFixLen.h"
#include <vector>
#include <chrono>
#include <iostream>

void benchmarkFFT() {

    const int bits = 16;
    const int size = 2 << (bits-1);

    ffft::FFTReal<float> fftDynamic(size);
    ffft::FFTRealFixLen<bits> fftFixed;

    std::vector<float> testVector(size, 0);
    std::vector<float> output(size, 0);

    for (auto& sample: testVector)
        sample = (rand() % 30000) / 30000.0f;

    auto bench = [&](auto& fft, auto name) {
        auto start = std::chrono::high_resolution_clock::now();
        fft.do_fft(&output[0], &testVector[0]);
        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        //std::cout << name << " spent " << durationMs << std::endl;
        return durationMs;
    };

    unsigned long dynCount = 0;
    unsigned long fixCount = 0;
    for (size_t i = 0; i < 1000; ++i) {

        for (auto& sample: testVector)
            sample = (rand() % 30000) / 30000.0f;

        dynCount += bench(fftDynamic, "Dyn");
        fixCount += bench(fftFixed, "Fix");
    }

    std::cout << "Total dyn: " << dynCount / 1000.0 << std::endl;
    std::cout << "Total fixed: " << fixCount / 1000.0 << std::endl;

    exit(0);
}


using namespace mtherapp;

int main(int argc, char *argv[])
{
    //benchmarkFFT();


    LogHandler::getInstance().setFilename("log.txt");
    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
