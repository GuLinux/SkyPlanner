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

using namespace std;

// STRUCT

  struct UGC {
    //  |name       |ra      |dec     |phot_mag|hubble_type|ugc_number|mcg          |poss_field|blue_major_axis|red_major_axis|class               |
    int index;
    std::string number;
    std::string ugcName;
    std::string mcgName;
    float magnitude;
    std::string hubbleType;
    int possField;
    float blueMajorAxis;
    float redMajorAxis;
    std::string galaxyClass;
  
	struct RightAscension {
        	int hours;
		int minutes;
		float seconds;
                float radians();
	};

    RightAscension ra;

	struct  Declination { 
		int degrees;
		float minutes;
                float radians();
	};

    Declination dec;
  };

// FUNZIONI
  
  float UGC::RightAscension::radians(){
	float hTot;
	hTot= static_cast<float>(hours) + ((minutes+(seconds/60.))/60.);
	float deg = 360.*hTot/24.;
	float rad = deg/180.*M_PI;
	return rad; 
}

float UGC::Declination::radians(){
  float degreesTot = static_cast<float>(degrees);
  int sign = degrees>0?1:-1;
  degreesTot += (minutes / 60. * sign);
  float rad = degreesTot/180.*M_PI;
  return rad;
}

  
int stringToInt(string);
float stringToFloat(string, float defaultValue = 0);
UGC::RightAscension stringToRightAscension(string right_ascension);
UGC::Declination stringToDeclination(string declination);
//PROGRAMMA

int main(int argc, char ** argv){
  
  QCoreApplication app(argc, argv);
  QStringList arguments = app.arguments();
  arguments.removeFirst();
  ifstream mgcInput(arguments.first().toStdString());
  vector<UGC> objects;
  string temp;

  getline(mgcInput, temp);
  int index = 0;
	while(mgcInput) {
          getline(mgcInput, temp);
          UGC object;
          object.index = index++;
                //PROBLEMI:
          //se NON c'è la magnitudine o la dimensione?
          //Nelle note va inserito anche The Mice, Grasshopper ecc -> fare colonna a parte nel file?
          
          
          //CAT NUMBER
          
          object.number = string(temp.begin() + 52, temp.begin() + 62);
          object.ugcName = string(temp.begin() + 1, temp.begin() + 12);
          //COMMON NAMES
          object.mcgName = string(temp.begin() + 63, temp.begin() + 76);
        
            //RIGHT ASCENSION
           string right_ascension(temp.begin() + 13, temp.begin() + 21);
           object.ra = stringToRightAscension(right_ascension);
                                  
          //DECLINATION
          string declination(temp.begin() + 22, temp.begin() + 30);
          object.dec = stringToDeclination(declination);
          
          //MAGNITUDE
          string magnitude(temp.begin() + 31, temp.begin() + 39);
          object.magnitude = stringToFloat(magnitude, 99);

          //SIZE
          string redMajorAxis(temp.begin() + 104, temp.begin() + 118);
          string blueMajorAxis(temp.begin() + 88, temp.begin() + 103);
          object.redMajorAxis = stringToFloat(redMajorAxis);
          object.blueMajorAxis = stringToFloat(blueMajorAxis);
          
          //NOTES (MORPHOLOGICAL TYPE)
//           string notes_type(temp.begin() + 38, temp.begin() + 52);
//           arp.notes_type = notes_type;
          object.galaxyClass = string(temp.begin() + 119, temp.begin() + 139);
          object.hubbleType = string(temp.begin() + 40, temp.begin() + 51);
          string possField = string(temp.begin() + 77, temp.begin() + 87);
          object.possField = stringToInt(possField);
        cerr << "object number: '" << object.number << "'" << endl;
        cerr << "object name: '" << object.ugcName<< "'" << endl;
        cerr << "object mgc: '" << object.mcgName<< "'" << endl;
        cerr << "object ra: '" << right_ascension << "'" << endl;
        cerr << "object dec: '" << declination << "'" << endl;
        cerr << "object mag: '" << magnitude<< "'" << endl;
        cerr << "object red_major_axis: '" << redMajorAxis<< "'" << endl;
        cerr << "object blue_major_axis: '" << blueMajorAxis<< "'" << endl;
        cerr << "object poss: '" << possField<< "'" << endl;
        cerr << "object galaxyClass: '" << object.galaxyClass<< "'" << endl;
        cerr << "object hubble_type: '" << object.hubbleType<< "'" << endl;
          objects.push_back(object);
          }
          
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
        CatalogsImporter importer("UGC", argc, argv);
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


UGC::Declination stringToDeclination(string declination){

	stringstream ingresso(declination);
	UGC::Declination temp;
	ingresso >> temp.degrees >> temp.minutes ;
	
	return temp;
}

UGC::RightAscension stringToRightAscension(string right_ascension){
	stringstream ingresso(right_ascension);
	UGC::RightAscension temp;
	ingresso >> temp.hours >> temp.minutes >> temp.seconds;
	return temp;
}
