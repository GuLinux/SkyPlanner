#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <vector>
#include <map>
#include <QTextStream>
#include <iostream>
#include <stdint.h>
#include <QDebug>
#include <QRegExp>
#include <QtSql/QSqlQuery>
#include <unordered_map>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

using namespace std;

struct ObjectName;
struct NgcObject {
    enum Catalogue {NGC, IC, Messier, Caldwell, ProperName};

    struct Key {
        Catalogue catalogue;
        int32_t number;
        QString toString() const { return QString("%1%2").arg(catalogue == NGC ? "NGC": "IC").arg(number); }
        bool operator==(const Key &other) const { return catalogue == other.catalogue && number == other.number; }
    };

  bool isIc;
  int32_t number;
  float ra, dec, mag, angularSize;
  uint32_t type;
  vector<ObjectName> otherNames;
  Catalogue catalogue() const { return isIc ? IC : NGC; }
  Key key() const { return { isIc ? IC : NGC, number }; }
};

map<NgcObject::Catalogue,QString> CatalogueNames {
    {NgcObject::NGC, "NGC"},
    {NgcObject::IC, "IC"},
    {NgcObject::Messier, "Messier"},
    {NgcObject::Caldwell, "Caldwell"},
    {NgcObject::ProperName, ""},
};

struct ObjectName {
    QString name;
    QString ngcNumber;
    QString comment;
    NgcObject::Key key() const;
    QString parsedName() const;
    NgcObject::Catalogue catalogue() const;
    int catalogueNumber() const;
};

NgcObject::Key ObjectName::key() const {
    NgcObject::Catalogue catalogue = NgcObject::NGC;
    QString number = ngcNumber;
    if(ngcNumber.startsWith("I")) {
        catalogue = NgcObject::IC;
        number = ngcNumber.mid(1);
    }
    return { catalogue, number.toInt() };
}

int ObjectName::catalogueNumber() const {
    if(catalogue() == NgcObject::Messier || catalogue() == NgcObject::Caldwell)
      return name.mid(1).toInt();
    return -1;
}

NgcObject::Catalogue ObjectName::catalogue() const {
    if(name.startsWith("C"))
        return NgcObject::Caldwell;
    if(name.startsWith("M"))
        return NgcObject::Messier;
    return NgcObject::ProperName;
}

QString ObjectName::parsedName() const {
  QRegExp transRx("_[(]\"(.*)\"[)]");
  if(!transRx.exactMatch(name))
      return QString(name).replace(QRegExp(" {2,10}"), " ");
  return transRx.cap(1);
}

namespace std
{
    template<>
    struct hash<NgcObject::Key>
    {
    public:
        std::size_t operator()(NgcObject::Key const& k) const
        {
            std::size_t h1 = std::hash<int>()(k.catalogue);
            std::size_t h2 = std::hash<int>()(k.number);
            return h1 ^  (h2 << 1);
         }
    };
}


ostream &operator <<(ostream &s, const NgcObject &ngc) {
    s << "{ " << (ngc.isIc ? "IC " : "NGC ") << ngc.number << ", ra: " << ngc.ra << ", dec: " << ngc.dec << ", magnitude: " << ngc.mag << ", angularSize: " << ngc.angularSize << ", type: " << ngc.type;
    s << ", otherNames: [";
    string separator;
    for(ObjectName name: ngc.otherNames) {
        s << separator << name.parsedName().toStdString();
        separator = ", ";
    }
    s << "] }";
  return s;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile file("/usr/share/stellarium/nebulae/default/ngc2000.dat");
    if(!file.open(QIODevice::ReadOnly)) {
        cerr << "Unable to open input file " << file.fileName().toStdString() << endl;
        return -1;
    }
    QDataStream ins(&file);
    unordered_map<NgcObject::Key,NgcObject> ngcObjects;

    ins.setVersion(QDataStream::Qt_4_5);

    while(! ins.atEnd()) {
        NgcObject ngc;
        ins >> ngc.isIc >> ngc.number >> ngc.ra >> ngc.dec >> ngc.mag >> ngc.angularSize >> ngc.type;
        ngcObjects[ngc.key()] = ngc;
    }
    file.close();
    QFile namesFile("/usr/share/stellarium/nebulae/default/ngc2000names.dat");
    if(!namesFile.open(QIODevice::ReadOnly)) {
        cerr << "Unable to open input file " << namesFile.fileName().toStdString() << endl;
        return -1;
    }
    QTextStream namesStream(&namesFile);
    QString line;
    while( (line = namesStream.readLine()) != QString()){
        QString ngcName = line.mid(37, 5).trimmed();
        QString catName = line.left(36).trimmed();
        QString comment = line.mid(42).trimmed();
        ObjectName objectName{catName, ngcName, comment};
//         qDebug() << "line: " << line;
//         qDebug() << "ngcName: " << ngcName << ", catName: " << catName << ", comment: " << comment;
//         qDebug() << "Loaded object " << CatalogueNames[objectName.catalogue()] << objectName.catalogueNumber() << " <<--->> " << objectName.key().toString() << "(orig: " << objectName.ngcNumber << ")";
        if(ngcObjects.count(objectName.key()) > 0) {
            ngcObjects[objectName.key()].otherNames.push_back(objectName);
        }
    }



    namesFile.close();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("ngc.sqlite");
    if(!db.open()) {
        cerr << "Unable to open database file!" << endl;
        return -1;
    }
    QSqlQuery createTables;
    if(!createTables.exec("CREATE TABLE objects (object_id TEXT PRIMARY KEY, ra REAL, dec REAL, magnitude REAL, angular_size REAL, type INTEGER);"))
        cerr << "Error creating table objects: " << createTables.lastError().text().toStdString() << endl;
    createTables.exec("TRUNCATE TABLE objects;");

    if(!createTables.exec("CREATE TABLE denominations (objects_object_id TEXT references objects(object_id), catalogue TEXT, number INTEGER, name TEXT, comment TEXT);"))
        cerr << "Error creating table denominations: " << createTables.lastError().text().toStdString() << endl;
    createTables.exec("TRUNCATE TABLE denominations;");

    int current = 0;
    QSqlDatabase::database().transaction();
    for(auto obj: ngcObjects ) {

        NgcObject ngc = obj.second;
        QSqlQuery addObject;
        addObject.prepare("INSERT INTO objects(object_id, ra, dec, magnitude, angular_size, type) VALUES(?, ?, ?, ?, ?, ?);");
        QString key = ngc.key().toString();
        auto addDenomination = [=] (const QString &catalogue, int catalogueNumber, const QString &name, const QString &comment) {
            QSqlQuery addDenomination;
            addDenomination.prepare("INSERT INTO denominations(objects_object_id, catalogue, number, name, comment) VALUES(?, ?, ?, ?, ?);");
            addDenomination.addBindValue(key);
            addDenomination.addBindValue(catalogue);
            addDenomination.addBindValue(catalogueNumber);
            addDenomination.addBindValue(name);
            addDenomination.addBindValue(comment);
            if(!addDenomination.exec())
              cerr << "Error adding denomination for " << catalogue.toStdString() << catalogueNumber << ": " << addDenomination.lastError().text().toStdString() << endl;
        };

        qDebug() << QString("%1").arg(current++, 5, 10, QChar('0')) << "/" << ngcObjects.size() << ": " << key ;
        addObject.addBindValue(key);
        addObject.addBindValue(ngc.ra);
        addObject.addBindValue(ngc.dec);
        addObject.addBindValue(ngc.mag);
        addObject.addBindValue(ngc.angularSize);
        addObject.addBindValue(ngc.type);
        addObject.exec();
        QString catalogueName = CatalogueNames[ngc.catalogue()];
        addDenomination(catalogueName, ngc.number, QString("%1 %2").arg(catalogueName).arg(ngc.number), "");
        for(ObjectName objectName: ngc.otherNames) {
            addDenomination(CatalogueNames[objectName.catalogue()], objectName.catalogueNumber(), objectName.parsedName(), objectName.comment);
        }
    }
    QSqlDatabase::database().commit();
    return 0;
}
