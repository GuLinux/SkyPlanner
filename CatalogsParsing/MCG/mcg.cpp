// Arp catalogue

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "models/ngcobject.h"
#include "../dbhelper.h"
#include "models/nebuladenomination.h"
#include <QCoreApplication>
#include <QStringList>

using namespace std;

// STRUCT

  struct MCG {
    int index;
    std::string number;
    std::string object_id;
    string other_names;
    float magnitude;
    float largest_dimension;
  
  
	struct RightAscension {
        	int hours;
		int minutes;
		float seconds;
                float radians();
	};

    RightAscension ra;

	struct  Declination { 
		int degrees;
		int minutes;
		int seconds;
                float radians();
	};

    Declination dec;

	
    string notes_type;

  };

// FUNZIONI
  
  float MCG::RightAscension::radians(){
	float hTot;
	hTot= static_cast<float>(hours) + ((minutes+(seconds/60.))/60.);
	float deg = 360.*hTot/24.;
	float rad = deg/180.*M_PI;
	return rad; 
}

float MCG::Declination::radians(){
	float degreesTot;
	degreesTot = static_cast<float>(degrees) + (minutes+(seconds/60.)/60.);
	float rad = degreesTot/180.*M_PI;
	return rad;
}

  
int stringToInt(string);
float stringToFloat(string, float defaultValue = 0);
MCG::RightAscension stringToRightAscension(string right_ascension);
MCG::Declination stringToDeclination(string declination);
//PROGRAMMA

int main(int argc, char ** argv){
  
  QCoreApplication app(argc, argv);
  QStringList arguments = app.arguments();
  arguments.removeFirst();
  ifstream mgcInput(arguments.first().toStdString());
  ifstream mgcFixMagInput(arguments[1].toStdString());
  vector<MCG> objects;
  map<QString, float> objectsWithMagnitudeOnly;
  string temp;
  
  while(mgcFixMagInput) {
    getline(mgcFixMagInput, temp);
    QString name = QString::fromStdString(string(temp.begin(), temp.begin() + 13)).trimmed();
    string magnitude(temp.begin() + 38, temp.begin() + 42);
    objectsWithMagnitudeOnly[name] = stringToFloat(magnitude, 99);
  }
  
  getline(mgcInput, temp);
  int index = 0;
	
	while(mgcInput) {
          getline(mgcInput, temp);
          MCG object;
          object.index = index++;
                //PROBLEMI:
          //se NON c'è la magnitudine o la dimensione?
          //Nelle note va inserito anche The Mice, Grasshopper ecc -> fare colonna a parte nel file?
          
          
          //ARP NUMBER
          object.number = string(temp.begin() + 1, temp.begin() + 15);
          
          //COMMON NAMES
          object.other_names = string(temp.begin() + 51, temp.begin() + 59);
        
            //RIGHT ASCENSION
           string right_ascension(temp.begin() + 16, temp.begin() + 23);
           object.ra = stringToRightAscension(right_ascension);
                                  
          //DECLINATION
          string declination(temp.begin() + 25, temp.begin() + 32);
          object.dec = stringToDeclination(declination);
          
          //MAGNITUDE
          string magnitude(temp.begin() + 43, temp.begin() + 49);
          object.magnitude = stringToFloat(magnitude, 99);
          if(object.magnitude == 0.0) {
            QString number = QString::fromStdString(object.number).trimmed();
            cerr << "WARNING: " << number.toStdString() << " magnitude is 0, fixing from other file: "
            << objectsWithMagnitudeOnly[number] << endl;
            object.number = objectsWithMagnitudeOnly[number];
          }

          //SIZE
          string largest_dimension(temp.begin() + 34, temp.begin() + 41);
          object.largest_dimension = stringToFloat(largest_dimension);
          
          //NOTES (MORPHOLOGICAL TYPE)
//           string notes_type(temp.begin() + 38, temp.begin() + 52);
//           arp.notes_type = notes_type;
            objects.push_back(object);
          }
          
          sort(objects.begin(), objects.end(), [](const MCG &a, const MCG &b) { return a.largest_dimension > b.largest_dimension; });
          
	 for(int i = 0; i< objects.size(); i++) {
           MCG &obj = objects[i];
           if(!obj.object_id.empty()) continue;
           char nextDupeId = 'B';
           obj.object_id = QString::fromStdString(obj.number).trimmed().toStdString();
           for(int j=i; j<objects.size(); j++) {
             
            MCG &obj1 = objects[j];
             if(obj1.object_id.empty() && obj1.index != obj.index && obj1.number == obj.number) {
               obj.object_id = QString("%1A").arg(QString::fromStdString(obj.number).trimmed()).toStdString();
               obj1.object_id = QString("%1%2").arg(QString::fromStdString(obj.number).trimmed()).arg(nextDupeId++).toStdString();
             }
           };
         };
        CatalogsImporter importer("MCG", argc, argv);
        for(MCG object: objects) {
          long long objectId = importer.findByCatalog(object.other_names);
          cerr << "inserting " << object.number << ", ngcic id=" << objectId;
          if(objectId < 0) {
            objectId = importer.insertObject(object.object_id, object.ra.radians(), object.dec.radians(), object.magnitude, object.largest_dimension, NgcObject::NebGx);
            if(objectId <= 0)
              throw runtime_error("Error inserting object");
            cerr << ", added new objectId: " << objectId;
          }
          //   long long insertDenomination(std::string catalogueNumber, const std::string &name, const std::string &comment, long long objectId, int searchMode, const std::string &other_catalogues = std::string());
          auto denId = importer.insertDenomination(object.number, object.number, "", objectId, NebulaDenomination::ByName, object.other_names);
          cerr << ": id=" << denId << endl;
        }
// 	for (int i=0;i<conta;i++){
//         long long otherId = importer.findByCatalog(array[i].other_names);
//           stringstream objectId;
//           objectId << "Arp " << array[i].arp_number;
//         if(otherId < 0) {
//           otherId = importer.insertObject(objectId.str(), array[i].ra.radians(), array[i].dec.radians(), array[i].magnitude, array[i].largest_dimension, NgcObject::NebGx);
//           if(otherId<=0)
//             throw std::runtime_error("Error inserting object");
//         }

  return 0;
}


// DEFINIZIONI FUNZIONI

int stringToInt(string arp_number){
	stringstream input(arp_number);
	int temp;
	input >> temp;
	
        return temp;	
}

float stringToFloat(string number, float defaultValue){
	
	stringstream ingresso(number);
	float temp = defaultValue;
	ingresso >> temp;
	
        return temp;	
}


MCG::Declination stringToDeclination(string declination){

	stringstream ingresso(declination);
	MCG::Declination temp;
	ingresso >> temp.degrees >> temp.minutes >> temp.seconds;
	
	return temp;
}

MCG::RightAscension stringToRightAscension(string right_ascension){
	stringstream ingresso(right_ascension);
	MCG::RightAscension temp;
	ingresso >> temp.hours >> temp.minutes >> temp.seconds;
	return temp;
}