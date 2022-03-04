#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

#define CONF_PARAM(z) Config::getInst().values[ z ] //TODO maybe replace with QSettings

#include <QObject>
#include <QAbstractTableModel>


namespace aurals {


    void setTestLocation(std::string newTL);
    void initGlobals();



    class Config : public QObject
    {

    public:
        bool *logs[10];
        std::string logsNames[10];
        int topIndex;

        std::unordered_map<std::string,std::string> values;
        void addLine(std::string anotherLine);

        double scaleCoef;
        double timeCoef;

        Q_INVOKABLE QString getValue(std::string v) {
            if (values.count(v))
                return values[v].c_str();
            return "";
        }

        std::string testsLocation;
        std::string invertedLocation;
        std::string theUserId;
        bool isMobile;
        struct ScreenSize
        {
            int width;
            int height;
            int dpi; //not set yet
            double scale;

        } screenSize;
        std::string platform;

    public:
       //Config() = default;
        virtual ~Config() = default;

        void connectLog(bool *ptrValue, int index=-1,std::string logName="unknown");

        void load(std::ifstream& file);
        void save(std::ofstream& file) const;

        void addValue(std::string name, std::string val);

        double getTimeCoef() const { return timeCoef; }
        void setTimeCoef(double newTC) { timeCoef = newTC; }

        double getScaleCoef() const { return scaleCoef; }
        void setScaleCoef(double scale) { scaleCoef = scale; }

        void printValues() const;
        void checkConfig();

        void cleanValues() { values.clear(); }


    public:
            static Config& getInst() {
                static Config instance;
                return instance;
            }

    public:
            Config(): topIndex(-1),scaleCoef(1.0),timeCoef(1) {}
            Config(const Config& root) = delete;
            Config& operator=(const Config&) = delete;

    };


    class ConfigQML : public QObject {

        Q_OBJECT

    public:
        ConfigQML(){}
        ConfigQML(QObject* parent) {}
        ~ConfigQML() = default;

        std::string parameter(std::string name) {
            return Config::getInst().getValue(name).toStdString();
        }

        Q_INVOKABLE QString param(QString name) {
            return Config::getInst().getValue(name.toStdString());
        }

    };

}


#endif // CONFIG_H
