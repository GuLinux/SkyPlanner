/*
 * 
 * SAC Red Stars Database Version 2.0
 * 
 * elaborazione lista
 * 
 * 
 */


#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>
#include "dbhelper.h"
#include "models/ngcobject.h"
#include "models/catalogue.h"
using namespace std;

// NAME |OTHER NAMES |CON|RA J2K |DecJ2K|V   |B-V |SPEC. |NOTES        

  struct RedStar{
    
    string name;
    string other_names;
    
	struct RA{
	  int hours;
	  float minutes;
	  float radians();
	  
	};
	
	
    RA rightAscension;
    
	
	
	struct DEC{
	  int degrees;
	  int minutes;
	  float radians();
	};
	
    DEC declination;
    float visual_magnitude;
    float B_V;
    string spectral_Type;
    string notes;
    
    
  };
  
  float RedStar::RA::radians(){
	float hTot;
	hTot= static_cast<float>(hours) + (minutes/60.);
	float deg = 360.*hTot/24.;
	float rad = deg/180.*M_PI;
	return rad; 
}
  
  float RedStar::DEC::radians(){
    float degreesTot;
          int sign = degrees>0?+1:-1;
    degreesTot = static_cast<float>(degrees) + (minutes/60.*sign);
    float rad = degreesTot/180.*M_PI;
    return rad;
}
  
  float stringToFloat(string);
  RedStar::DEC stringToDeclination(string);
  RedStar::RA stringToRightAscension(string);


int main(int argc, char **argv){
  QCoreApplication app(argc, argv);
  QStringList arguments = app.arguments();
  arguments.removeFirst();
  int number_of_stars = 333;
  
  RedStar * list;
  list = new RedStar[number_of_stars];
  
  ifstream upload;
  upload.open(arguments.first().toStdString());
    if(!upload){
      cout << "File non trovato"<< endl;
      return -1;
    }
  
  for (int i = -1 ; i < number_of_stars ; i++){
    
    
    string ale;
    
    getline (upload,ale);
    
    if (i>=0){
    
    RedStar redstar;
    string name(ale.begin() + 1, ale.begin() + 14);
			redstar.name = name;
			
			//cout <<redstar.name << endl;
			
    string other_names(ale.begin() + 15, ale.begin() + 50);
			redstar.other_names = other_names;
			
			//cout << redstar.other_names<<endl;
			
   string rightAscension(ale.begin() + 55, ale.begin() + 62);
			redstar.rightAscension = stringToRightAscension(rightAscension);
			
    string declination(ale.begin() + 63, ale.begin() + 69);
			redstar.declination = stringToDeclination(declination);			
           
			
    string visual_magnitude(ale.begin() + 70, ale.begin() + 74);
			redstar.visual_magnitude = stringToFloat(visual_magnitude);
			
			//cout << redstar.visual_magnitude << endl;
			
    string B_V(ale.begin() + 75, ale.begin() + 79);
			redstar.B_V = stringToFloat(B_V);
			
			// cout << redstar.B_V << endl;
			
    string spectral_Type(ale.begin() + 80, ale.begin() + 86);
			redstar.spectral_Type = spectral_Type;
			
    string notes(ale.begin()+ 87, ale.begin()+ 147);
			redstar.notes = notes;
   
    list[i]= redstar;
    
    cout << " Star Name: " << list[i].name 
	 << " Other names: " << list[i].other_names
	 << " Right ascension: " << list[i].rightAscension.hours << " " << list[i].rightAscension.minutes 
         << " - as radians: " <<  list[i].rightAscension.radians() 
	 << " Declination: " << list[i].declination.degrees << " " << list[i].declination.minutes 
         << " - as radians: " << list[i].declination.radians()
	 << " Vistal magnitude: " << list[i].visual_magnitude
	 << " B-V index: " << list[i].B_V
	 << " Spectral type: " << list[i].spectral_Type
	 << " Other notes: " << list[i].notes
	 << endl;
    }
  }
  
  upload.close();
  
  cout << "WITH LOVE." << endl;
  
  string saguaroCat("Saguaro Astronomy Club Red Stars");
  CatalogsImporter importer(saguaroCat, "SAC_RED_STARS", app);
  cout << "Import starting...." << endl;
  for(int i=0; i<number_of_stars; i++) {
    importer.setCatalogue(QString::fromStdString(saguaroCat), "SAC_RED_STARS");
    RedStar star = list[i];
    QString catNumber = QString::number(i+1);

    QString objectId = QString("SAC_REDSTAR_%1").arg(i+1);
    auto ngcObjectId = importer.insertObject(objectId.toStdString(), star.rightAscension.radians(), star.declination.radians(), star.visual_magnitude, -1, NgcObject::RedStar);
    importer.insertDenomination(catNumber.toStdString(), star.name, star.notes, ngcObjectId, Catalogue::ByName, star.other_names);
    if(QString::fromStdString(star.other_names).trimmed().isEmpty())
      continue;
    importer.setCatalogue(QString());
    for(QString otherDenomination: QString::fromStdString(star.other_names).trimmed().split(";", QString::SkipEmptyParts)) {
      importer.insertDenomination(string(), otherDenomination.toStdString(), string(), ngcObjectId, Catalogue::ByName, star.other_names);
    }
  }
}

float stringToFloat(string number){
	stringstream upload(number);
	float temp;
	upload >> temp;
	
        return temp;	
}


RedStar::RA stringToRightAscension(string rightAscension){

	stringstream upload(rightAscension);
	RedStar::RA temp;
	upload >> temp.hours >> temp.minutes;
	
	return temp;
}


RedStar::DEC stringToDeclination(string declination){

	stringstream upload(declination);
	RedStar::DEC temp;
	upload >> temp.degrees >> temp.minutes;
	
	return temp;
}

