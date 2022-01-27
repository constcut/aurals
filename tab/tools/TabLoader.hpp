#ifndef TABLOADER_H
#define TABLOADER_H

#include <memory>

#include "tab/Tab.hpp"


namespace aural_sight {


    class GTabLoader
    {
    public:
        bool open(std::string fileName);
        std::unique_ptr<Tab>& getTab() { return tab; }
    private:
        std::unique_ptr<Tab> tab;
    };

}


#endif // TABLOADER_H
