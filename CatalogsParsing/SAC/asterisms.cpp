// ASTERISMI

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;


	struct Asterism {

	string name;

	struct RightAscension {
        	int  hours;
		float minutes;
                float radians() const;
	};

        RightAscension ra;

	struct  Declination { 
		int degrees;
		int minutes;
                float radians() const;
	};

	Declination dec;

	float magnitude;	
	
	string size;
	
	string notes;

 	};

float parseSize(const string &size);

float Asterism::RightAscension::radians() const {
	float hTot;
	hTot= static_cast<float>(hours) + (minutes/60.);
	float deg = 360.*hTot/24.;
	float rad = deg/180.*M_PI;
	return rad; 
}

float Asterism::Declination::radians() const {
	float degreesTot;
	degreesTot = static_cast<float>(degrees) + (minutes/60.);
	float rad = degreesTot/180.*M_PI;
	return rad;
}

float stringToFloat(string);
Asterism::RightAscension stringToRightAscension(string);
Asterism::Declination stringToDeclination(string);
string insert(const Asterism &asterism, int index);

int main(){

	fstream input;
	input.open("SAC_Asterisms_Ver32_Fence.txt");
  if(!input) {
    cerr << "Error! Input file SAC_Asterisms_Ver32_Fence.txt missing" << endl;
    return 1;
   }
	
	int conta=0;
	while(!input.eof()){
        string temp;
	getline(input, temp);
	conta++; 
	}
	conta -= 3;

	input.close();

	input.open("SAC_Asterisms_Ver32_Fence.txt");
	
	Asterism * array;
	array = new Asterism[conta];
	
	for(int i=-2; i< conta ; i++ ) {
		string temp;
		getline(input, temp);
		if(i>=0) {
			Asterism asterism;
                        string name(temp.begin() + 1, temp.begin() + 23);
			asterism.name = boost::trim_copy(name);
						
			string notes(temp.begin() + 68, temp.begin() + 210);
			asterism.notes = boost::trim_copy(notes);

			string magnitude(temp.begin() + 43, temp.begin() + 47);
			asterism.magnitude = stringToFloat(magnitude);
			
			string ar(temp.begin() + 28, temp.begin() + 35);
			asterism.ra = stringToRightAscension(ar);
						
			string decl(temp.begin() + 36, temp.begin() + 42);
			asterism.dec = stringToDeclination(decl);			
           
                        string size(temp.begin() + 48, temp.begin() + 59);
                        asterism.size = size;

			array[i]= asterism;
        		
			 
		}
	}
	
cout << "BEGIN TRANSACTION;" << endl;
        
	for(int i=0;i<conta;i++){
	cerr << "Asterismo n. " << i+1 << ": nome=" << array[i].name 
             << ", magnitudine =" << array[i].magnitude
             << ", right ascension: " << array[i].ra.hours << " " << array[i].ra.minutes 
             << " - as radians: " <<  array[i].ra.radians() 
             << ", declination: " << array[i].dec.degrees << " " << array[i].dec.minutes 
             << " - as radians: " << array[i].dec.radians()
             << ", size = " << parseSize(array[i].size)
             << ", comments = " << array[i].notes
             << endl; 
             cout << insert(array[i], i+1) << endl;
	}

cout << "END TRANSACTION;" << endl;
return 0;

}


string sql(string s) {
  boost::replace_all(s, "'", "''");
  return s;
}

string insert(const Asterism &asterism, int index) {
           stringstream objectName;
           objectName << "SAC_ASTERISM_" << index;
           stringstream o;

           o << "INSERT INTO objects(\"object_id\", ra, \"dec\", magnitude, angular_size, type ) VALUES('" << objectName.str() << "', " << asterism.ra.radians()
            << ", " << asterism.dec.radians()
            << ", " << asterism.magnitude
            << ", " << parseSize(asterism.size)
            << ", 10);" << endl;
           o << "INSERT INTO denominations(catalogue, \"number\", name, comment, objects_id) VALUES("
            << "'Saguaro Astronomy Club Asterisms', " << index << ", '" << sql(asterism.name) << "', '" << sql(asterism.notes) << "', (select id from objects where object_id = '" << objectName.str() << "'));" << endl;
           return o.str();
}

float stringToFloat(string magnitude){
	stringstream ingresso(magnitude);
	float temp;
	ingresso >> temp;
	
        return temp;	
}

Asterism::RightAscension stringToRightAscension(string ar){
        // 01 22.4
	stringstream ingresso(ar);
	Asterism::RightAscension temp;
	ingresso >> temp.hours >> temp.minutes;
	
	return temp;
}

float parseSizeElement(const std::string &s) {
  if(s.empty()) return -1;
  float temp;
  stringstream ss(s);
  ss >> temp;
  if(s.find("'") != string::npos) {
    temp /= 60.;
  }
  return temp;
}

float parseSize(const string &size) {
  stringstream s(size);
  float first = -10;
  float second = -10;

  string a;
  while(s) {
    string b;
    s >> b;
    if(b == "X") {
      a = "";
    } else
      a += b;
    if(first<0)
      first = parseSizeElement(a);
    else
      second = parseSizeElement(a);
  }
  return max(first, second);
}

Asterism::Declination stringToDeclination(string dec){

	stringstream ingresso(dec);
	Asterism::Declination temp;
	ingresso >> temp.degrees >> temp.minutes;
	
	return temp;
}

