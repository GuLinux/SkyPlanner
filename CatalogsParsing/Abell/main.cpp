#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <iostream>
#include "models/ngcobject.h"
using namespace std;


double h2rad(const QString &h)
{
  int hours = h.split(" ").first().toInt();
  double minutes = h.split(" ").last().toDouble();
  double deg = static_cast<double>(hours) + (minutes / 60);
  deg = 360. * deg / 24;
  return deg * M_PI / 180.;
}

double deg2rad(const QString &deg)
{
  int degrees = deg.split(" ").first().toInt();
  double minutes = deg.split(" ").last().toDouble();
  double d = static_cast<double>(degrees) + (minutes/60);
  return d * M_PI / 180.;
}

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QFile file (app.arguments()[1]);
  file.open(QIODevice::ReadOnly);
  QTextStream s(&file);
  QTextStream o(stdout);

  const QString objQuery = "INSERT INTO objects (object_id, \"ra\", \"dec\", magnitude, angular_size, type) VALUES('Abell%1', %2, %3, %4, %5, %6);";
  const QString nameQuery = "INSERT INTO denominations(\"catalogue\", \"number\", \"name\", \"comment\", objects_id) VALUES('%1', %2, '%3','%4', %5);";

  while(!s.atEnd()) {
    QString first = s.readLine();
    QString second = s.readLine();
    //qDebug() << "First Line:" << first;
    //qDebug() << "Second Line:" << second;
    QString catNumber = first.left(5).trimmed();
    QString ar = second.mid(6, 7);
    QString dec = second.mid(14, 6);
    double magnitude = second.mid(64, 4).toDouble();
    double radRA = h2rad(ar);
    double radDec = deg2rad(dec);
    o << objQuery.arg(catNumber).arg(radRA).arg(radDec).arg(magnitude).arg(-1).arg(NgcObject::NebGalCluster) << endl;
    o << nameQuery.arg("Abell").arg(catNumber).arg(QString("Abell %1").arg(catNumber)).arg("").arg(QString("(SELECT id from objects WHERE object_id = 'Abell%1')").arg(catNumber)) << endl;
  }
}
