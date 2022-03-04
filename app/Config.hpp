#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>
#include <iterator>

#define CONF_PARAM(z) Config::getInst().values[ z ] //TODO maybe replace with QSettings

#include <QObject>
#include <QAbstractTableModel>
#include <QDebug>


namespace aurals {


    void setTestLocation(std::string newTL);
    void initGlobals();



    class Config : public QObject
    {

    public: //private
        bool *logs[10];
        std::string logsNames[10];
        int topIndex;

        std::map<std::string,std::string> values;
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

        static Config& getInst() {
            static Config instance;
            return instance;
        }

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



    class ConfigTableModel : public QAbstractTableModel
    {
        Q_OBJECT
        //QML_ELEMENT
        //QML_ADDED_IN_MINOR_VERSION(1)

    public:
        int rowCount(const QModelIndex & = QModelIndex()) const override
        {
            return Config::getInst().values.size();
        }

        int columnCount(const QModelIndex & = QModelIndex()) const override
        {
            return 2;
        }

        QVariant data(const QModelIndex &index, int role) const override
        {
            if (role !=  Qt::DisplayRole)
                return QVariant();

            auto beginIt = Config::getInst().values.begin();
            auto it = std::next(beginIt, index.row());

            if (index.column() == 0)
                return it->first.c_str();

            if (index.column() == 1)
                return it->second.c_str();

            return QVariant();
        }

        QHash<int, QByteArray> roleNames() const override
        {
            return { {Qt::DisplayRole, "display"} };
        }

        Q_INVOKABLE void updateValue(int row, QString value) {
            auto beginIt = Config::getInst().values.begin();
            auto it = std::next(beginIt, row);
            it->second = value.toStdString();
            qDebug() << "Updated: " << it->first.c_str() << " to "
                     << it->second.c_str();
        }
    };

}


#endif // CONFIG_H
