#include "app/Init.hpp"
#include "app/LogHandler.hpp"


#include "libs/fft/FFTReal.h"
#include "libs/fft/FFTRealFixLen.h"

#include "libs/fft/FFTunreal.hpp"
//#include "libs/fft/FFTurealfix.hpp"

#include <vector>
#include <chrono>
#include <iostream>

void benchmarkFFT() {

    const int bits = 16;
    const int size = 2 << (bits-1);

    ffft::FFTReal<float> fftDynamic(size);
    ffft::FFTRealFixLen<bits> fftFixed;

    FFTReal<float> fftUn(size);
    //FFTRealFixLen<float, bits> fftUnf;

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
    unsigned long unCount = 0;
    unsigned long unfCount = 0;



    bench(fftDynamic, "Dyn");
    std::vector<float> output2(output.begin(), output.end());

    output = std::vector<float>(size, 0);
    bench(fftUn, "UN");

    for (size_t i = 0; i < size; ++i) {
        if (output[i] != output2[i]) {
            qDebug() << "I suck :(  " << output[i] << " " << output2[i];
            break;
        }
    }

    /*
    bench(fftFixed, "Dyn");
    output2 = std::vector<float>(output.begin(), output.end());

    output = std::vector<float>(size, 0);
    bench(fftUnf, "UN");

    for (size_t i = 0; i < size; ++i) {
        if (output[i] != output2[i]) {
            qDebug() << "Fix I suck :(  " << output[i] << " " << output2[i];
            break;
        }
    }

    exit(0);*/


    for (size_t i = 0; i < 10000; ++i) {

        for (auto& sample: testVector)
            sample = (rand() % 30000) / 30000.0f;


        dynCount += bench(fftDynamic, "Dyn");
        fixCount += bench(fftFixed, "Fix");
        unCount += bench(fftUn, "UN");
        //unfCount += bench(fftUnf, "UNf");

    }

    qDebug() << "Total dyn: " << dynCount / 1000.0;
    qDebug() << "Total fixed: " << fixCount / 1000.0;
    qDebug() << "Total un: " << unCount / 1000.0;
    //qDebug() << "Total un fixed: " << unCount / 1000.0;

}


using namespace mtherapp;

int main(int argc, char *argv[])
{

    LogHandler::getInstance().setFilename("log.txt");

    benchmarkFFT();
    exit(0);

    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
