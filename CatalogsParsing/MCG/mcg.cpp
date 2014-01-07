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
	float degreesTot = static_cast<float>(degrees);
  float minutesTot = static_cast<float>(minutes);
  degreesTot += minutesTot / 60.;
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
    QString name = QString::fromStdString(string(temp.begin(), temp.begin() + 13)).trimmed().toUpper();
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
          object.other_names = string(temp.begin() + 51, temp.begin() + 60);
            //RIGHT ASCENSION
           string right_ascension(temp.begin() + 16, temp.begin() + 24);
           object.ra = stringToRightAscension(right_ascension);
                                  
          //DECLINATION
          string declination(temp.begin() + 25, temp.begin() + 33);
          object.dec = stringToDeclination(declination);
          
          //MAGNITUDE
          string magnitude(temp.begin() + 43, temp.begin() + 50);
          object.magnitude = stringToFloat(magnitude, 99);
          if(object.magnitude == 0.0) {
            QString number = QString::fromStdString(object.number).trimmed().toUpper();
            float magnitudeFixed = objectsWithMagnitudeOnly.count(number) > 0 ? objectsWithMagnitudeOnly[number] : 99;
            cerr << "WARNING: " << number.toStdString() << " magnitude is 0, fixing from other file: " << magnitudeFixed << endl;
            object.magnitude = magnitudeFixed;
          }

          //SIZE
          string largest_dimension(temp.begin() + 34, temp.begin() + 42);
          object.largest_dimension = stringToFloat(largest_dimension);
        cerr << "object number: '" << object.number << "'" << endl;
        cerr << "object other names: '" << object.other_names<< "'" << endl;
        cerr << "object ra: '" << right_ascension << "'" << endl;
        cerr << "object dec: '" << declination << "'" << endl;
        cerr << "object mag: '" << magnitude<< "'" << endl;
        cerr << "object largest_dimension: '" << largest_dimension<< "'" << endl;
          //NOTES (MORPHOLOGICAL TYPE)
//           string notes_type(temp.begin() + 38, temp.begin() + 52);
//           arp.notes_type = notes_type;
            objects.push_back(object);
          }
          
          sort(objects.begin(), objects.end(), [](const MCG &a, const MCG &b) { return a.largest_dimension > b.largest_dimension; });
          
	 for(int i = 0; i< objects.size(); i++) {
           MCG &obj = objects[i];
           if(!obj.object_id.empty()) continue;
           QString objectNum = QString::fromStdString(obj.number).toUpper().trimmed();
           char nextDupeId = 'B';
           obj.object_id = QString::fromStdString(obj.number).trimmed().toStdString();
           for(int j=i; j<objects.size(); j++) {
            MCG &obj1 = objects[j];
             
            QString object1Num = QString::fromStdString(obj1.number).toUpper().trimmed();
             if(obj1.object_id.empty() && obj1.index != obj.index && object1Num == objectNum) {
               obj.object_id = objectNum.append('A').toStdString();
               obj1.object_id = QString("%1%2").arg(object1Num).arg(nextDupeId++).toStdString();
             }
           };
         };
         
        sort(objects.begin(), objects.end(), [](const MCG &a, const MCG &b) { return a.object_id > b.object_id; });
        if(arguments.contains("--pretend")) {
          for(MCG &obj: objects) {
            cout << "object " << obj.number << " [" << obj.object_id 
              << ", magnitude=" << obj.magnitude 
              << endl;
          }
          return 0;
        }
        CatalogsImporter importer("MCG", argc, argv);
        index = 0;
        for(MCG object: objects) {
          long long objectId = importer.findByCatalog(object.other_names);
          cerr << "inserting " << object.number << "[" << index++ << "/" << objects.size() << "], ngcic id=" << objectId;
          if(objectId < 0) {
            objectId = importer.insertObject(object.object_id, object.ra.radians(), object.dec.radians(), object.magnitude, object.largest_dimension / 60., NgcObject::NebGx);
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
	ingresso >> temp.degrees >> temp.minutes ;
	
	return temp;
}

MCG::RightAscension stringToRightAscension(string right_ascension){
	stringstream ingresso(right_ascension);
	MCG::RightAscension temp;
	ingresso >> temp.hours >> temp.minutes >> temp.seconds;
	return temp;
}
