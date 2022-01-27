#include "TabLoader.hpp"

#include <fstream>
#include <QDebug>

#include "tab/tools/GmyFile.hpp"
#include "tab/tools/GtpFiles.hpp"


using namespace aural_sight;


bool GTabLoader::open(std::string fileName)
{
    std::ifstream file(fileName, std::ios::binary);

    if (file.is_open() == false) {
        qDebug() << "Failed to open file from loader";
        return false;
    }

    char firstBytes[3]={0};
    file.read(firstBytes,1);

    if (firstBytes[0] == 'G')
    {
        file.read(&firstBytes[1],1);
        if (firstBytes[1]=='A')
        {
            GmyFile gF;
            tab = std::make_unique<Tab>();
            if (gF.loadFromFile(file,tab.get(),true))
            {
                tab->connectTracks();
                file.close();
                return true;
            }
        }
    }
    else
    {
        file.read(firstBytes,2);
        firstBytes[2] = 0;

        std::string headBytes = firstBytes;
        if (headBytes=="FI")
        {
            char version[2];
            char skip[2];

            for (size_t i = 0; i < 28; ++i) //1+29-2 + post
            {
                if (i==18) file.read(version,1);
                else
                if (i==20) file.read(&version[1],1);
                else
                    file.read(skip,1);
            }

            std::uint8_t versionIndex = version[0]-48;
            if (version[1]!='0')
            {
                versionIndex *= 10;
                versionIndex += version[1]-48;
            }

            if ((versionIndex == 5) || (versionIndex == 51)
                    || (versionIndex == 4) || (versionIndex == 3))
            {
                tab = std::make_unique<Tab>();
            }

            bool loaded = false;

            if ((versionIndex == 5) || (versionIndex == 51))
            {
                Gp5Import gImp;
                try {
                    loaded = gImp.import(file,tab.get(),versionIndex);
                }
                catch(...) {
                    loaded = false;
                }
            }
            if (versionIndex == 4)
            {
                Gp4Import gImp;
                try {
                    loaded = gImp.import(file,tab.get(),versionIndex);
                }
                catch(...) {
                    loaded = false;
                }
            }
            if (versionIndex == 3)
            {
                Gp3Import gImp;
                try {
                    loaded = gImp.import(file,tab.get(),versionIndex);
                }
                catch(...) {
                    loaded = false;
                }
            }
            if (loaded)
            {
                tab->postLoading();
                tab->connectTracks();

                file.close();
                return true;
            }
        }
        else
        {
            //qDebug() << "unknown format";
            file.close();
            return false;
        }

    }

    file.close();
    return false;
}
