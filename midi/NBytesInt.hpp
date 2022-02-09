#ifndef NBYTESINT_H
#define NBYTESINT_H

#include <list>
#include <deque>
#include <fstream>

namespace aurals {

    class NBytesInt : public std::deque<uint8_t> {

    public:
        NBytesInt(){}
        NBytesInt(uint32_t source);

        uint32_t readFromFile(std::ifstream& f);
        uint32_t writeToFile(std::ofstream& f) const;

        uint32_t getValue() const;
    };
}

#endif
