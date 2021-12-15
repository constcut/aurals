#ifndef NBYTESINT_H
#define NBYTESINT_H



#include <list>
#include <deque>
#include <fstream>

namespace mtherapp {

    class NBytesInt : public std::deque<std::uint8_t> {

    public:
        NBytesInt(){}
        NBytesInt(std::uint32_t source);

        std::uint32_t readFromFile(std::ifstream& f);
        std::uint32_t writeToFile(std::ofstream& f);

        std::uint32_t getValue();
    };

}

#endif
