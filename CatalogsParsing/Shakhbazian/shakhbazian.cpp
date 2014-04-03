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
  int number = -1;
  Angle rightAscension;
  Angle declination;
  double magR = 99;
  double magV = 99;
  QString notes;
  QString object;
  double magnitude() const;
  QString json() const;
  bool isInteresting() const;
};

double ShakhbazianGalaxy::magnitude() const
{
  return min(magR, magV);
}

bool ShakhbazianGalaxy::isInteresting() const
{
  return (!object.isEmpty() || number > 0) && (magnitude() < 99 || !notes.isEmpty());
}



QString ShakhbazianGalaxy::json() const
{
  return QString(R"({ "index": %1, "group": %2, "number"=%3, "object": "%4", "rightAscension": %5, "declination": %6, "magR": %7, "magV": %8, "notes": "%9"})")
    .arg(index)
    .arg(group)
    .arg(number)
    .arg(object)
    .arg(rightAscension.degrees())
    .arg(declination.degrees())
    .arg(magR)
    .arg(magV)
    .arg(notes)
    ;
}


struct ShakhbazianGroup {
  int group;
  vector<ShakhbazianGalaxy> galaxies;
  Angle rightAscension() const;
  Angle declination() const;
  Angle size() const;
  double magnitude() const;
  QString description() const;
  QString json() const;
};

QString ShakhbazianGroup::json() const
{
  QStringList out;
  transform(begin(galaxies), end(galaxies), back_inserter(out), [](const ShakhbazianGalaxy &g) { return g.json(); });
  return QString("[ %1 ]").arg(out.join(", "));
}


Angle ShakhbazianGroup::declination() const
{
  Angle angle;
  for(auto galaxy: galaxies)
    angle += galaxy.declination;
  angle /= galaxies.size();
  return angle;
}

Angle ShakhbazianGroup::rightAscension() const
{
  Angle angle;
  for(auto galaxy: galaxies)
    angle += galaxy.rightAscension;
  angle /= galaxies.size();
  return angle;
}

Angle ShakhbazianGroup::size() const
{
  Angle minAR = rightAscension();
  Angle maxAR = rightAscension();
  Angle minDEC = declination();
  Angle maxDEC = declination();
  for(auto galaxy: galaxies) {
    minAR = min(minAR, galaxy.rightAscension);
    minDEC = min(minDEC, galaxy.declination);
    maxAR = max(maxAR, galaxy.rightAscension);
    maxDEC = max(maxDEC, galaxy.declination);
  }
  Angle maxDiff = max(maxAR - minAR, maxDEC - minDEC);
  return maxDiff * 1.5;
}

QString ShakhbazianGroup::description() const
{
  int n = 0;
  QString _d = QString("%1 galaxies.\n").arg(galaxies.size());
  for(auto galaxy: galaxies) {
    if(!galaxy.isInteresting()) continue;
    if(n++>15) break;
    _d += QString("Galaxy \"%1\": magnitude %2, notes/other names: %3\n")
      .arg(galaxy.object.isEmpty() ? QString::number(galaxy.number) : galaxy.object)
      .arg(galaxy.magnitude() >= 99 ? "N/A" : QString::number(galaxy.magnitude()) )
      .arg(galaxy.notes.isEmpty() ? "N/A" : galaxy.notes)
    ;
  }
  return _d.trimmed();
}


double ShakhbazianGroup::magnitude() const
{
  double magnitude = 99;
  for(auto galaxy: galaxies)
    magnitude = min(magnitude, galaxy.magnitude() );
  return magnitude;
}

ostream &operator<<(ostream &o, const ShakhbazianGalaxy &galaxy) {
  o << "{ " << galaxy.index << ", group: " << galaxy.group << ", object: " << galaxy.object.toStdString() << ", group #: " 
    << galaxy.number << ", ar: " << galaxy.rightAscension.printable(Angle::Hourly) << ", dec: " << galaxy.declination.printable() 
    << ", mag: " << galaxy.magnitude() << ", notes: " << galaxy.notes.toStdString() << " }";
  return o;
}

ostream &operator<<(ostream &o, const ShakhbazianGroup &group) {
  o << "Shakhbazian #" << group.group << ": magnitude=" << group.magnitude() << ", ar=" << group.rightAscension().printable(Angle::Hourly) << ", dec=" << group.declination().printable() 
    << ", size=" << group.size().printable() << endl;
  o << "Description: " << group.description().toStdString() << endl;
  for(auto galaxy: group.galaxies)
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
  map<int, ShakhbazianGroup> groups;
  do {
    line = input.readLine();
    if(line.isEmpty() || line.left(1) == "#") continue;
    QStringList values = line.split("|");
    ShakhbazianGalaxy galaxy;
    galaxy.index = values[2].toInt();
    galaxy.group = values[3].toInt();
    galaxy.object = values[4].trimmed();
    if(!values[5].trimmed().isEmpty())
      galaxy.number = values[5].toInt();
    galaxy.rightAscension = Angle::degrees(values[0].toDouble());
    galaxy.declination = Angle::degrees(values[1].toDouble());
    if(!values[10].trimmed().isEmpty())
      galaxy.magV = values[10].toDouble();
    if(!values[11].trimmed().isEmpty())
      galaxy.magR = values[11].toDouble();
    galaxy.notes = values[14].trimmed();
    groups[galaxy.group].group = galaxy.group;
    groups[galaxy.group].galaxies.push_back(galaxy);
  } while(!line.isNull());

  for(auto group: groups) {
    cout << group.second << endl;
  }
  CatalogsImporter importer(app);
  QSqlQuery insertCatalog(importer.db);
  insertCatalog.exec(R"(INSERT INTO catalogues ("version", "name", "code", "priority", "search_mode", "hidden") VALUES (0, 'Shakhbazian' , 'shk' , -92, 1, 0))");
  QSqlQuery getCatalogId(R"(SELECT id FROM catalogues WHERE code = 'shk')", importer.db);
  getCatalogId.exec();
  if(!getCatalogId.next())
    throw runtime_error("Error adding catalogue");
  long long catalogId = getCatalogId.value(0).toLongLong();
  for(auto group: groups) {
    QString objectId = QString("shk %1");
    QSqlQuery insertObject(R"(INSERT INTO "objects"
      ( object_id, ra, "dec", magnitude, angular_size, type, extra_data )
      VALUES ( :object_id, :ra, :dec, :magnitude, :angular_size, :type, :extra_data ) )", importer.db);
      insertObject.bindValue(":object_id", objectId.arg(group.first));
      insertObject.bindValue(":ra", group.second.rightAscension().radians());
      insertObject.bindValue(":dec", group.second.declination().radians());
      insertObject.bindValue(":magnitude", group.second.magnitude());
      insertObject.bindValue(":angular_size", group.second.size().degrees());
      insertObject.bindValue(":type", NgcObject::NebGalGroups);
      insertObject.bindValue(":extra_data", group.second.json() );
    insertObject.exec();
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


