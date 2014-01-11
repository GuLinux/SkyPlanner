#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <iostream>
#include "models/ngcobject.h"
#include "models/catalogue.h"
#include "dbhelper.h"
using namespace std;


double h2rad(const QString &h)
{
  int hours = h.split(" ").first().trimmed().toInt();
  double minutes = h.split(" ").last().trimmed().toDouble();
  double deg = static_cast<double>(hours) + (minutes / 60.);
  deg = 360. * deg / 24;
  return deg * M_PI / 180.;
}

double deg2rad(const QString &deg)
{
  int degrees = deg.split(" ").first().trimmed().toInt();
  int sign = degrees>0?+1:-1;
  double minutes = deg.split(" ").last().trimmed().toDouble();
  double d = static_cast<double>(degrees) + (minutes/60.*sign);
  return d * M_PI / 180.;
}

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  QFile file (app.arguments()[1]);
  file.open(QIODevice::ReadOnly);
  QTextStream s(&file);
  QTextStream o(stdout);
  CatalogsImporter importer("Abell", app);

  o << "BEGIN TRANSACTION;" << endl;
  const QString objQuery = "INSERT INTO objects (object_id, \"ra\", \"dec\", magnitude, angular_size, type) VALUES('Abell %1', %2, %3, %4, %5, %6);";
  const QString nameQuery = "INSERT INTO denominations(\"catalogue\", \"number\", \"name\", \"comment\", objects_id) VALUES('%1', '%2', '%3','%4', %5);";
  QString prefix = file.fileName().contains("POOR.CLU") ? "S" : "";
  while(!s.atEnd()) {
    QString first = s.readLine();
    QString second = s.readLine();
    //qDebug() << "First Line:" << first;
    //qDebug() << "Second Line:" << second;
    QString catNumber = first.left(5).trimmed();
    QString ar = second.mid(6, 7).trimmed();
    QString dec = second.mid(14, 6).trimmed();
    double magnitude = second.mid(64, 4).toDouble();
    double radRA = h2rad(ar);
    double radDec = deg2rad(dec);
    qDebug() << "Adding " << catNumber;
    auto objectId = importer.insertObject(catNumber, radRA, radDec, magnitude, -1, NgcObject::NebGalCluster);
    if(objectId <= 0 )
      throw std::runtime_error("Error adding object to database");
    o << objQuery.arg(catNumber).arg(radRA).arg(radDec).arg(magnitude).arg(-1).arg(NgcObject::NebGalCluster) << endl;
    QString objectName = QString("Abell %1").arg(catNumber.replace(" ", ""));
    importer.insertDenomination(catNumber, objectName, "", objectId, Catalogue::ByName );
    o << nameQuery.arg("Abell").arg(catNumber).arg(objectName).arg("").arg(QString("(SELECT id from objects WHERE object_id = 'Abell %1')").arg(catNumber)) << endl;
  }
  o << "END TRANSACTION;" << endl;
}

