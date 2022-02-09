#ifndef GMYFILE_H
#define GMYFILE_H

#include "../Tab.hpp"

namespace aurals {

    class GmyFile
    {
    public:
        GmyFile();

        bool saveToFile(std::ofstream& file, Tab *tab);
        bool loadFromFile(std::ifstream& file, Tab *tab, bool skipVersion=false);

    private:

        bool loadString(std::ifstream& file, std::string &strValue);
        bool saveString(std::ofstream& file, std::string &strValue);
    };

}

#endif // GMYFILE_H
