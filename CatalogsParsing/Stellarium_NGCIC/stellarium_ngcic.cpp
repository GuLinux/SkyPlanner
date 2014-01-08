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
#include "dbhelper.h"

using namespace std;
namespace {

enum Catalogue {NGC, IC, Messier, Caldwell, ProperName};
struct Key {
    Catalogue catalogue;
    int32_t number;
    QString toString() const { return QString("%1%2").arg(catalogue == NGC ? "NGC": "IC").arg(number); }
    bool operator==(const Key &other) const { return catalogue == other.catalogue && number == other.number; }
};
struct ObjectName {
    QString name;
    QString ngcNumber;
    QString comment;
    Key key() const;
    QString parsedName() const;
    Catalogue catalogue() const;
    int catalogueNumber() const;
};

struct NgcObject {
  bool isIc;
  int32_t number;
  float ra, dec, mag, angularSize;
  uint32_t type;
  vector<ObjectName> otherNames;
  Catalogue catalogue() const { return isIc ? IC : NGC; }
  Key key() const { return { isIc ? IC : NGC, number }; }
};
map<Catalogue,QString> CatalogueNames {
    {NGC, "NGC"},
    {IC, "IC"},
    {Messier, "Messier"},
    {Caldwell, "Caldwell"},
    {ProperName, ""},
};

}

Key ObjectName::key() const {
    Catalogue catalogue = NGC;
    QString number = ngcNumber;
    if(ngcNumber.startsWith("I")) {
        catalogue = IC;
        number = ngcNumber.mid(1);
    }
    return { catalogue, number.toInt() };
}

int ObjectName::catalogueNumber() const {
    if(catalogue() == Messier || catalogue() == Caldwell)
      return name.mid(1).toInt();
    return -1;
}

Catalogue ObjectName::catalogue() const {
    if(name.startsWith("C"))
        return Caldwell;
    if(name.startsWith("M"))
        return Messier;
    return ProperName;
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
    struct hash<Key>
    {
    public:
        std::size_t operator()(Key const& k) const
        {
            std::size_t h1 = std::hash<int>()(k.catalogue);
            std::size_t h2 = std::hash<int>()(k.number);
            return h1 ^  (h2 << 1);
         }
    };
}

namespace {
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
    unordered_map<Key,NgcObject> ngcObjects;

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
    CatalogsImporter importer("", a);

    int current = 0;
    for(auto obj: ngcObjects ) {

        NgcObject ngc = obj.second;
        QString key = ngc.key().toString();
        auto objectId = importer.insertObject(key, ngc.ra, ngc.dec, ngc.mag, ngc.angularSize, ngc.type);
        if(objectId <= 0)
          throw std::runtime_error("Error inserting ngc object");

        auto addDenomination = [=,&importer] (const QString &catalogue, int catalogueNumber, const QString &name, const QString &comment) {
            importer.setCatalogue(catalogue);
            qDebug() << "Adding object: catalogue=" << catalogue << "number=" << catalogueNumber << "name=" << name << "comment=" << comment;
            importer.insertDenomination(catalogueNumber <= 0 ? QString() : QString::number(catalogueNumber), name, comment, objectId, 0);
        };

        qDebug() << QString("%1").arg(current++, 5, 10, QChar('0')) << "/" << ngcObjects.size() << ": " << key ;
        QString catalogueName = CatalogueNames[ngc.catalogue()];
        addDenomination(catalogueName, ngc.number, QString("%1 %2").arg(catalogueName).arg(ngc.number), "");
        for(ObjectName objectName: ngc.otherNames) {
            addDenomination(CatalogueNames[objectName.catalogue()], objectName.catalogueNumber(), objectName.parsedName(), objectName.comment);
        }
    }
    return 0;
}
