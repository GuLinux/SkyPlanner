// Arp catalogue

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "models/ngcobject.h"
#include "../dbhelper.h"
#include "models/nebuladenomination.h"

using namespace std;

// STRUCT

  struct Arp {
    
    int arp_number;
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
  
  float Arp::RightAscension::radians(){
	float hTot;
	hTot= static_cast<float>(hours) + ((minutes+(seconds/60.))/60.);
	float deg = 360.*hTot/24.;
	float rad = deg/180.*M_PI;
	return rad; 
}

float Arp::Declination::radians(){
	float degreesTot;
	degreesTot = static_cast<float>(degrees) + (minutes+(seconds/60.)/60.);
	float rad = degreesTot/180.*M_PI;
	return rad;
}
  
int stringToInt(string);
float stringToFloat(string, float defaultValue = 0);
Arp::RightAscension stringToRightAscension(string);
Arp::Declination stringToDeclination(string);
void ordinaVettore (Arp array[], int dim);

//PROGRAMMA

int main(int argc, char ** argv){
  
  
  ifstream upload;
  
  upload.open("arp.dat");
  
  int conta = 0;
  
  while (!upload.eof()){
      string temp;
      getline(upload, temp);
      conta++; 
  }
 
  
  upload.close();

  upload.open("arp.dat");
	
	Arp* array;
	array = new Arp[conta];
	
	for(int i=0; i< conta ; i++ ) {
	  
	        string temp;
		getline(upload, temp);
		
			Arp arp;
			
			      //PROBLEMI:
			//se NON c'è la magnitudine o la dimensione?
			//Nelle note va inserito anche The Mice, Grasshopper ecc -> fare colonna a parte nel file?
			
			
			//ARP NUMBER
                        string arp_number(temp.begin() + 0, temp.begin() + 3);
			arp.arp_number = stringToInt(arp_number);
			
			//COMMON NAMES
			string other_names(temp.begin() + 4, temp.begin() + 19);
			arp.other_names = other_names;
			
			//MAGNITUDE
			string magnitude(temp.begin() + 20, temp.begin() + 24);
			arp.magnitude = stringToFloat(magnitude, 99);
			
			//RIGHT ASCENSION
			string right_ascension(temp.begin() + 57, temp.begin() + 67);
			arp.ra = stringToRightAscension(right_ascension);
						
			//DECLINATION
			string declination(temp.begin() + 68, temp.end());
			arp.dec = stringToDeclination(declination);	
			
			//SIZE
                        string largest_dimension(temp.begin() + 27, temp.begin() + 30);
                        arp.largest_dimension = stringToFloat(largest_dimension);
			
			//NOTES (MORPHOLOGICAL TYPE)
                        string notes_type(temp.begin() + 38, temp.begin() + 52);
			arp.notes_type = notes_type;
			
			
			array[i]= arp;
        		
			 
		}
	
	
	 upload.close();
	 
	 ordinaVettore (array, conta);
        CatalogsImporter importer("Arp", argc, argv);
	for (int i=0;i<conta;i++){
        long long otherId = importer.findByCatalog(array[i].other_names);
          stringstream objectId;
          objectId << "Arp " << array[i].arp_number;
        if(otherId < 0) {
          otherId = importer.insertObject(objectId.str(), array[i].ra.radians(), array[i].dec.radians(), array[i].magnitude, array[i].largest_dimension, NgcObject::NebGx);
          if(otherId<=0)
            throw std::runtime_error("Error inserting object");
        }
        stringstream notes;
        notes << array[i].notes_type << "; other catalogue: " << array[i].other_names;
        importer.insertDenomination(array[i].arp_number, objectId.str(), notes.str(), otherId, NebulaDenomination::ByCatalog, array[i].other_names);
        cerr << "Found arp " << array[i].arp_number << " with other_names " << array[i].other_names << " in db: " << otherId << endl;
	cout << "Arp n. " << array[i].arp_number 
	     << ", other names: " << array[i].other_names
             << ", magnitude: " << array[i].magnitude
             << ", right ascension: " << array[i].ra.hours << " " << array[i].ra.minutes << " " << array[i].ra.seconds
             << " - as radians: " <<  array[i].ra.radians() 
             << ", declination: " << array[i].dec.degrees << " " << array[i].dec.minutes << " " << array[i].dec.seconds
             << " - as radians: " << array[i].dec.radians()
             << ", size: " << array[i].largest_dimension
             << ", morphological type: " << array[i].notes_type
             << endl; 
  
	 }
  
  
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


Arp::Declination stringToDeclination(string declination){

	stringstream ingresso(declination);
	Arp::Declination temp;
	ingresso >> temp.degrees >> temp.minutes >> temp.seconds;
	
	return temp;
}

Arp::RightAscension stringToRightAscension(string right_ascension){
	stringstream ingresso(right_ascension);
	Arp::RightAscension temp;
	ingresso >> temp.hours >> temp.minutes >> temp.seconds;
	return temp;
}

void ordinaVettore (Arp array[], int dim){
	Arp appo;
	int posMax;

	for(int j=0; j< dim; j++) {
	    posMax=j;
	    for(int i=j; i<dim; i++){
		if (array[i].arp_number < array[posMax].arp_number)
                posMax = i;
        }		
	appo= array[posMax];

	array[posMax]=array[j];
	array[j]= appo;
	}
}