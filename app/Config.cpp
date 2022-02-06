#include "Config.hpp"

#include <string>
#include <fstream>
#include <QDebug>
#include <iostream>


using namespace aural_sight;


void aural_sight::initGlobals()
{
    AConfig& conf = AConfig::getInst();

    conf.invertedLocation = "";
    conf.testsLocation = "";
    conf.theUserId = "";

    conf.screenSize.height=0;
    conf.screenSize.width=0;
    conf.screenSize.dpi=0;

    conf.isMobile = false; //desktop actually is default

#ifdef __ANDROID_API__
    conf.isMobile = true;
#endif
    conf.platform = "other";
#ifdef linux
    conf.platform = "linux";  //not very sure
#endif
#ifdef WIN32
   conf.platform = "windows";
#endif
#ifdef __ANDROID_API__
   conf.platform = "android";
#endif

}


void aural_sight::setTestLocation(std::string newTL)
{
    std::string invertedLocation="";

    std::cout << "setCurrent location to "<<newTL.c_str()<<std::endl;
    AConfig& conf = AConfig::getInst();

    conf.testsLocation = newTL;

    //invert
    invertedLocation.clear();

    for (size_t i = 0; i < newTL.length(); ++i)
        if (newTL[i]=='/')
            invertedLocation.push_back('\\');
        else
            invertedLocation.push_back(newTL[i]);

    conf.invertedLocation = invertedLocation;
}


void AConfig::connectLog(bool *ptrValue, int index,std::string logName)
{
    if (index==-1)
        index = topIndex+1;

    logs[index] = ptrValue;
    logsNames[index] = logName;

    if (index>topIndex)
        topIndex=index;
}



//log Name 1/0
//v Name 1290391
void AConfig::load(std::ifstream &file)
{
    std::string lastLine = "firstOne";

    int count = 0;
    while (!lastLine.empty())
    {
        ++count;
        std::string line;
        std::getline(file, line);


        if (line != lastLine)
        {
            addLine(line);
            lastLine = line;
        }
        else
            break;
        //hotfix
        if (line.substr(0,4)=="zend")
            break;
    }

    std::cout <<"Total count "<<count;


}

std::string repairString(std::string str)
{
    std::string response = str;
    //first 0
    while (response[0]==' ')
        response.erase(0,1);



    if (response[0]=='"')
    {
        int clIndex = 0;
        for (size_t i = 1; i < response.size(); ++i)
            if (response[i]=='"')
            {
                clIndex = i;
                break;
            }

        if (clIndex)
        response = response.substr(1,clIndex-1);
    }


    return response;
}

void AConfig::addLine(std::string anotherLine)
{
            //std::cout <<anotherLine.c_str()<<std::endl;
            size_t eqSign = anotherLine.find("=");

            if (eqSign != std::string::npos)
            {
                std::string left = anotherLine.substr(0,eqSign-1);
                std::string right = anotherLine.substr(eqSign+1);
                right = repairString(right);
                values[left]=right;
                std::cout <<left.c_str()<<"="<<right.c_str()<<std::endl;
            }
}

//should connect some values to be used
//first
void AConfig::save(std::ofstream &file) const
{
     //file.opened();
     for (auto it = values.begin();
          it!= values.end(); ++it)
     {
         std::string curLine = (*it).first + " = " + (*it).second + "\n";
         file.write(curLine.c_str(), curLine.size());
     }
     file.close();
}

void AConfig::printValues() const
{
    qDebug()<<"Configuration parameters";
    for (auto it = values.begin();
         it!= values.end(); ++it)
    {
        qDebug()<< (*it).first.c_str() <<" = "<<(*it).second.c_str();
    }
    qDebug()<<"Config printed";
}

 void AConfig::addValue(std::string name, std::string val)
 {
    if (values.count(name) == 0)
    {
        std::pair<std::string,std::string> newLine;
        newLine.first = name;
        newLine.second = val;
        values.insert(newLine);
       // std::map<std::string,std::string>:
    }
 }

void AConfig::checkConfig()
{
    addValue("TabView.genAMusic","g");
    addValue("TabView.genMidi","h");
    addValue("TrackView.nextBeat",">>>");
    addValue("TrackView.prevBeat","<<<");
    addValue("TrackView.nextBar","x");
    addValue("TrackView.prevBar","z");
    addValue("TrackView.stringUp","^^^");
    addValue("TrackView.stringDown","vvv");
    addValue("TrackView.playAMusic","spc");
    addValue("TrackView.playMidi","ent");

    addValue("TrackView.setPause","p");
    addValue("TrackView.deleteNote","del");
    addValue("TrackView.increaceDuration","q");
    addValue("TrackView.decreaceDuration","w");
    addValue("TrackView.save","s");
    addValue("TrackView.quickOpen","y");
    addValue("Main.open","open");
    addValue("colors.background","white");
    addValue("colors.panBG","gray");
    addValue("colors.curBar","black");
    addValue("colors.curBeat","gray");
    addValue("colors.curString","darkgray"); //hmm
    addValue("colors.default","gray");
    addValue("colors.exceed","darkred");
    addValue("colors.curTrack","blue");
    addValue("colors.selection","darkgreen");

    addValue("colors.combo.background","gray");
    addValue("colors.combo.pressed","darkgray");
    addValue("colors.combo.focus","white");
    addValue("colors.combo.hover","darkblue");

    addValue("colors.button.background","gray");
    addValue("colors.button.pressed","darkgray");
    addValue("colors.button.focus","white");
    addValue("colors.button.hover","darkblue");

    addValue("colors.button.bgspecial","darkgray");


    addValue("effects.vibrato","vib");
    addValue("effects.slide","sli");
    addValue("effects.hammer","ham");
    addValue("effects.letring","l.r.");
    addValue("effects.palmmute","p.m.");
    addValue("effects.harmonics","harm");
    addValue("effects.trill","trill");
    addValue("effects.tremolo","trem");
    addValue("effects.stokatto","stok");
    addValue("effects.fadein","f In");
    addValue("effects.accent","acc");
    addValue("TrackView.alwaysShowBarSign","0");

    addValue("gestures","classic");

    addValue("TrackView.largeNotes","1");

    addValue("fullscreen","1");

    //bottom
    addValue("toolBar","0");   //top bottom right left 0
    addValue("mainMenu","0");    //0 or else
    addValue("pannels","oneline");//tabbed docked classic(cutten yet) oneline

    addValue("invertImages","0");

    addValue("statusBar","1");
    addValue("metronome","0");

    //addValue("crashOnPlayHotFix","1"); //to remember the leak
    //if not found sepparae buttons

    addValue("logs.tab","0");
    addValue("logs.midi","0");
    addValue("logs.abstractmusic","0");
    addValue("logs.ownformat","0");
    addValue("logs.othersformats","0");

    addValue("images","1");

    addValue ("crashAutoSend","0");
    addValue ("addRootCrashes","1");
    addValue ("sdcardLogDebug","0");//debug

    addValue("iconsSet","1");
    addValue("turnPinchZoomOn","0");

    addValue("tapAndHoldCopy","0");

    addValue("skipTabView","1");

    addValue("showStatusViewHint","0");

    addValue("upsideDownNotes","1");
    addValue("showNotesView","0");

    addValue("mergeMidiTracks","0");

    addValue("zend","ofconfig");

    addValue("onloadBaseCheck","1");

    addValue("midi.config","on");
}


