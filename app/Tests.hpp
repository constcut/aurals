#ifndef TESTS_H
#define TESTS_H

#include <string>

namespace aurals {

    class Config;

    int getTime();
    bool testScenario();
    void connectConfigs(Config& config);
    bool midiPrint(std::string fileName);
    bool greatCheckScenarioCase(uint32_t scen, uint32_t from, uint32_t to, uint32_t v);

}

#endif // TESTS_H
