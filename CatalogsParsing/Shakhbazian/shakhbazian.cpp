// Arp catalogue

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "models/ngcobject.h"
#include "../dbhelper.h"
#include "models/catalogue.h"
#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include "types.h"

using namespace std;

// STRUCT

struct ShakhbazianGalaxy {
  int index;
  int group;
  int number;
  Angle rightAscension;
  Angle declination;
  double magnitude;
  QString notes;
};

ostream &operator<<(ostream &o, const ShakhbazianGalaxy &galaxy) {
  o << "{ " << galaxy.index << ", group: " << galaxy.group << ", group #: " << galaxy.number << ", ar: " << galaxy.rightAscension.printable(Angle::Hourly) << ", dec: " << galaxy.declination.printable() << ", mag: " << galaxy.magnitude << ", notes: " << galaxy.notes.toStdString() << " }";
  return o;
}

ostream &operator<<(ostream &o, const pair<int, vector<ShakhbazianGalaxy>> group) {
  o << "Shakhbazian #" << group.first << endl;
  for(auto galaxy: group.second)
    o << "\t" << galaxy << endl;
  return o;
}


int main(int argc, char ** argv){
  
  QCoreApplication app(argc, argv);
  QStringList arguments = app.arguments();
  arguments.removeFirst();
  QFile inputFile(arguments.first());
  inputFile.open(QFile::ReadOnly);
  QTextStream input(&inputFile);
  QString line;
  map<int, vector<ShakhbazianGalaxy>> groups;
  do {
    line = input.readLine();
    if(line.isEmpty() || line.left(1) == "#") continue;
    QStringList values = line.split("|");
    ShakhbazianGalaxy galaxy;
    galaxy.index = values[2].toInt();
    galaxy.group = values[3].toInt();
    galaxy.number = values[5].toInt();
    galaxy.rightAscension = Angle::degrees(values[0].toDouble());
    galaxy.declination = Angle::degrees(values[1].toDouble());
    galaxy.magnitude = 99;
    for(auto i: vector<int>{10, 11})
      if(galaxy.magnitude >= 99 && !values[i].trimmed().isEmpty())
        galaxy.magnitude = values[i].trimmed().toDouble();
    galaxy.notes = values[14].trimmed();
    groups[galaxy.group].push_back(galaxy);
  } while(!line.isNull());

  for(auto group: groups) {
    cout << group << endl;
  }
  return 0;        
/*
        if(arguments.contains("--pretend")) {
          for(UGC &obj: objects) {
            cout << "object " << obj.number << " [" << obj.ugcName << "], magnitude=" << obj.magnitude << ", mcg=" << obj.mcgName 
            << ", ra=" << obj.ra.hours << " " << obj.ra.minutes << " " << obj.ra.seconds << ", rad=" << obj.ra.radians()
            << ", dec=" << obj.dec.degrees << " " << obj.dec.minutes << ", rad=" << obj.dec.radians()
            << ", size=" << max(obj.blueMajorAxis, obj.redMajorAxis) / 60.
            << endl;
          }
          return 0;
        }
        CatalogsImporter importer("UGC", "UGC", argc, argv);
        index = 0;
        for(UGC object: objects) {
          long long objectId = importer.findBy("SELECT objects_id from denominations WHERE \
            lower(denominations.catalogue)  = :catalogue AND \
            lower(denominations.\"number\" ) like '%'||:number||'%' \
            ", {
              {":catalogue", "mcg"},
              {":number",  QString::fromStdString(object.mcgName).trimmed().toLower()},
            });
          cerr << "inserting " << object.number << "[" << index++ << "/" << objects.size() << "], MCG id=" << objectId;
          if(objectId < 0) {
            object.mcgName = string("NOT FOUND: ") + object.mcgName;
            objectId = importer.insertObject(object.ugcName, object.ra.radians(), object.dec.radians(), object.magnitude, 
                                             max(object.redMajorAxis, object.blueMajorAxis) / 60., NgcObject::NebGx);
            if(objectId <= 0)
              throw runtime_error("Error inserting object");
            cerr << ", added new objectId: " << objectId;
          }
          cerr << endl;
            QString notes = QString("Galaxy Class: %1; Hubble Type: %2; poss field: %3")
              .arg(QString::fromStdString(object.galaxyClass).toLower().trimmed())
              .arg(QString::fromStdString(object.hubbleType).trimmed())
              .arg(object.possField);
          //   long long insertDenomination(std::string catalogueNumber, const std::string &name, const std::string &comment, long long objectId, int searchMode, const std::string &other_catalogues = std::string());
          auto denId = importer.insertDenomination(object.number, object.ugcName, notes.toStdString(), objectId, Catalogue::ByCatalog, object.mcgName);
        }
  return 0;
*/
}


