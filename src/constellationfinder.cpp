/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 *  Adapted from: http://djm.cc/constellation.js
 * 
 */
// David Moews, 2008-VI-14; revised 2008-VIII-2

/*
 * precess() and get_name() are adapted from program.c, from
 * ftp://cdsarc.u-strasbg.fr/pub/cats/VI/42/program.c
 * which is from
 * CDS (Centre de donnees astronomiques de Strasbourg) catalog VI/42; also see
 *
 * Identification of a constellation from a position,
 * Nancy G. Roman,
 * PUBLICATIONS OF THE ASTRONOMICAL SOCIETY OF THE PACIFIC,
 * 99 (July 1987), pp. 695-699.
 *
 * Program.c says:
 * This program is a translation with a few adaptations of the 
 * Fortran program.f, made by FO @ CDS  (francois@simbad.u-strasbg.fr)  
 * in November 1996.
 */
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include "constellationfinder.h"
#include <vector>
using namespace std;
namespace {
struct CoordinatesElement {
  double lAR;
  double uAR;
  double lDec;
  string abbr;
};

vector<CoordinatesElement> coordinatesTable {
{0.0000,24.0000,88.0000,"UMi"},
{8.0000,14.5000,86.5000,"UMi"},
{21.0000,23.0000,86.1667,"UMi"},
{18.0000,21.0000,86.0000,"UMi"},
{0.0000,8.0000,85.0000,"Cep"},
{9.1667,10.6667,82.0000,"Cam"},
{0.0000,5.0000,80.0000,"Cep"},
{10.6667,14.5000,80.0000,"Cam"},
{17.5000,18.0000,80.0000,"UMi"},
{20.1667,21.0000,80.0000,"Dra"},
{0.0000,3.5083,77.0000,"Cep"},
{11.5000,13.5833,77.0000,"Cam"},
{16.5333,17.5000,75.0000,"UMi"},
{20.1667,20.6667,75.0000,"Cep"},
{7.9667,9.1667,73.5000,"Cam"},
{9.1667,11.3333,73.5000,"Dra"},
{13.0000,16.5333,70.0000,"UMi"},
{3.1000,3.4167,68.0000,"Cas"},
{20.4167,20.6667,67.0000,"Dra"},
{11.3333,12.0000,66.5000,"Dra"},
{0.0000,0.3333,66.0000,"Cep"},
{14.0000,15.6667,66.0000,"UMi"},
{23.5833,24.0000,66.0000,"Cep"},
{12.0000,13.5000,64.0000,"Dra"},
{13.5000,14.4167,63.0000,"Dra"},
{23.1667,23.5833,63.0000,"Cep"},
{6.1000,7.0000,62.0000,"Cam"},
{20.0000,20.4167,61.5000,"Dra"},
{20.5367,20.6000,60.9167,"Cep"},
{7.0000,7.9667,60.0000,"Cam"},
{7.9667,8.4167,60.0000,"UMa"},
{19.7667,20.0000,59.5000,"Dra"},
{20.0000,20.5367,59.5000,"Cep"},
{22.8667,23.1667,59.0833,"Cep"},
{0.0000,2.4333,58.5000,"Cas"},
{19.4167,19.7667,58.0000,"Dra"},
{1.7000,1.9083,57.5000,"Cas"},
{2.4333,3.1000,57.0000,"Cas"},
{3.1000,3.1667,57.0000,"Cam"},
{22.3167,22.8667,56.2500,"Cep"},
{5.0000,6.1000,56.0000,"Cam"},
{14.0333,14.4167,55.5000,"UMa"},
{14.4167,19.4167,55.5000,"Dra"},
{3.1667,3.3333,55.0000,"Cam"},
{22.1333,22.3167,55.0000,"Cep"},
{20.6000,21.9667,54.8333,"Cep"},
{0.0000,1.7000,54.0000,"Cas"},
{6.1000,6.5000,54.0000,"Lyn"},
{12.0833,13.5000,53.0000,"UMa"},
{15.2500,15.7500,53.0000,"Dra"},
{21.9667,22.1333,52.7500,"Cep"},
{3.3333,5.0000,52.5000,"Cam"},
{22.8667,23.3333,52.5000,"Cas"},
{15.7500,17.0000,51.5000,"Dra"},
{2.0417,2.5167,50.5000,"Per"},
{17.0000,18.2333,50.5000,"Dra"},
{0.0000,1.3667,50.0000,"Cas"},
{1.3667,1.6667,50.0000,"Per"},
{6.5000,6.8000,50.0000,"Lyn"},
{23.3333,24.0000,50.0000,"Cas"},
{13.5000,14.0333,48.5000,"UMa"},
{0.0000,1.1167,48.0000,"Cas"},
{23.5833,24.0000,48.0000,"Cas"},
{18.1750,18.2333,47.5000,"Her"},
{18.2333,19.0833,47.5000,"Dra"},
{19.0833,19.1667,47.5000,"Cyg"},
{1.6667,2.0417,47.0000,"Per"},
{8.4167,9.1667,47.0000,"UMa"},
{0.1667,0.8667,46.0000,"Cas"},
{12.0000,12.0833,45.0000,"UMa"},
{6.8000,7.3667,44.5000,"Lyn"},
{21.9083,21.9667,44.0000,"Cyg"},
{21.8750,21.9083,43.7500,"Cyg"},
{19.1667,19.4000,43.5000,"Cyg"},
{9.1667,10.1667,42.0000,"UMa"},
{10.1667,10.7833,40.0000,"UMa"},
{15.4333,15.7500,40.0000,"Boo"},
{15.7500,16.3333,40.0000,"Her"},
{9.2500,9.5833,39.7500,"Lyn"},
{0.0000,2.5167,36.7500,"And"},
{2.5167,2.5667,36.7500,"Per"},
{19.3583,19.4000,36.5000,"Lyr"},
{4.5000,4.6917,36.0000,"Per"},
{21.7333,21.8750,36.0000,"Cyg"},
{21.8750,22.0000,36.0000,"Lac"},
{6.5333,7.3667,35.5000,"Aur"},
{7.3667,7.7500,35.5000,"Lyn"},
{0.0000,2.0000,35.0000,"And"},
{22.0000,22.8167,35.0000,"Lac"},
{22.8167,22.8667,34.5000,"Lac"},
{22.8667,23.5000,34.5000,"And"},
{2.5667,2.7167,34.0000,"Per"},
{10.7833,11.0000,34.0000,"UMa"},
{12.0000,12.3333,34.0000,"CVn"},
{7.7500,9.2500,33.5000,"Lyn"},
{9.2500,9.8833,33.5000,"LMi"},
{0.7167,1.4083,33.0000,"And"},
{15.1833,15.4333,33.0000,"Boo"},
{23.5000,23.7500,32.0833,"And"},
{12.3333,13.2500,32.0000,"CVn"},
{23.7500,24.0000,31.3333,"And"},
{13.9583,14.0333,30.7500,"CVn"},
{2.4167,2.7167,30.6667,"Tri"},
{2.7167,4.5000,30.6667,"Per"},
{4.5000,4.7500,30.0000,"Aur"},
{18.1750,19.3583,30.0000,"Lyr"},
{11.0000,12.0000,29.0000,"UMa"},
{19.6667,20.9167,29.0000,"Cyg"},
{4.7500,5.8833,28.5000,"Aur"},
{9.8833,10.5000,28.5000,"LMi"},
{13.2500,13.9583,28.5000,"CVn"},
{0.0000,0.0667,28.0000,"And"},
{1.4083,1.6667,28.0000,"Tri"},
{5.8833,6.5333,28.0000,"Aur"},
{7.8833,8.0000,28.0000,"Gem"},
{20.9167,21.7333,28.0000,"Cyg"},
{19.2583,19.6667,27.5000,"Cyg"},
{1.9167,2.4167,27.2500,"Tri"},
{16.1667,16.3333,27.0000,"CrB"},
{15.0833,15.1833,26.0000,"Boo"},
{15.1833,16.1667,26.0000,"CrB"},
{18.3667,18.8667,26.0000,"Lyr"},
{10.7500,11.0000,25.5000,"LMi"},
{18.8667,19.2583,25.5000,"Lyr"},
{1.6667,1.9167,25.0000,"Tri"},
{0.7167,0.8500,23.7500,"Psc"},
{10.5000,10.7500,23.5000,"LMi"},
{21.2500,21.4167,23.5000,"Vul"},
{5.7000,5.8833,22.8333,"Tau"},
{0.0667,0.1417,22.0000,"And"},
{15.9167,16.0333,22.0000,"Ser"},
{5.8833,6.2167,21.5000,"Gem"},
{19.8333,20.2500,21.2500,"Vul"},
{18.8667,19.2500,21.0833,"Vul"},
{0.1417,0.8500,21.0000,"And"},
{20.2500,20.5667,20.5000,"Vul"},
{7.8083,7.8833,20.0000,"Gem"},
{20.5667,21.2500,19.5000,"Vul"},
{19.2500,19.8333,19.1667,"Vul"},
{3.2833,3.3667,19.0000,"Ari"},
{18.8667,19.0000,18.5000,"Sge"},
{5.7000,5.7667,18.0000,"Ori"},
{6.2167,6.3083,17.5000,"Gem"},
{19.0000,19.8333,16.1667,"Sge"},
{4.9667,5.3333,16.0000,"Tau"},
{15.9167,16.0833,16.0000,"Her"},
{19.8333,20.2500,15.7500,"Sge"},
{4.6167,4.9667,15.5000,"Tau"},
{5.3333,5.6000,15.5000,"Tau"},
{12.8333,13.5000,15.0000,"Com"},
{17.2500,18.2500,14.3333,"Her"},
{11.8667,12.8333,14.0000,"Com"},
{7.5000,7.8083,13.5000,"Gem"},
{16.7500,17.2500,12.8333,"Her"},
{0.0000,0.1417,12.5000,"Peg"},
{5.6000,5.7667,12.5000,"Tau"},
{7.0000,7.5000,12.5000,"Gem"},
{21.1167,21.3333,12.5000,"Peg"},
{6.3083,6.9333,12.0000,"Gem"},
{18.2500,18.8667,12.0000,"Her"},
{20.8750,21.0500,11.8333,"Del"},
{21.0500,21.1167,11.8333,"Peg"},
{11.5167,11.8667,11.0000,"Leo"},
{6.2417,6.3083,10.0000,"Ori"},
{6.9333,7.0000,10.0000,"Gem"},
{7.8083,7.9250,10.0000,"Cnc"},
{23.8333,24.0000,10.0000,"Peg"},
{1.6667,3.2833,9.9167,"Ari"},
{20.1417,20.3000,8.5000,"Del"},
{13.5000,15.0833,8.0000,"Boo"},
{22.7500,23.8333,7.5000,"Peg"},
{7.9250,9.2500,7.0000,"Cnc"},
{9.2500,10.7500,7.0000,"Leo"},
{18.2500,18.6622,6.2500,"Oph"},
{18.6622,18.8667,6.2500,"Aql"},
{20.8333,20.8750,6.0000,"Del"},
{7.0000,7.0167,5.5000,"CMi"},
{18.2500,18.4250,4.5000,"Ser"},
{16.0833,16.7500,4.0000,"Her"},
{18.2500,18.4250,3.0000,"Oph"},
{21.4667,21.6667,2.7500,"Peg"},
{0.0000,2.0000,2.0000,"Psc"},
{18.5833,18.8667,2.0000,"Ser"},
{20.3000,20.8333,2.0000,"Del"},
{20.8333,21.3333,2.0000,"Equ"},
{21.3333,21.4667,2.0000,"Peg"},
{22.0000,22.7500,2.0000,"Peg"},
{21.6667,22.0000,1.7500,"Peg"},
{7.0167,7.2000,1.5000,"CMi"},
{3.5833,4.6167,0.0000,"Tau"},
{4.6167,4.6667,0.0000,"Ori"},
{7.2000,8.0833,0.0000,"CMi"},
{14.6667,15.0833,0.0000,"Vir"},
{17.8333,18.2500,0.0000,"Oph"},
{2.6500,3.2833,-1.7500,"Cet"},
{3.2833,3.5833,-1.7500,"Tau"},
{15.0833,16.2667,-3.2500,"Ser"},
{4.6667,5.0833,-4.0000,"Ori"},
{5.8333,6.2417,-4.0000,"Ori"},
{17.8333,17.9667,-4.0000,"Ser"},
{18.2500,18.5833,-4.0000,"Ser"},
{18.5833,18.8667,-4.0000,"Aql"},
{22.7500,23.8333,-4.0000,"Psc"},
{10.7500,11.5167,-6.0000,"Leo"},
{11.5167,11.8333,-6.0000,"Vir"},
{0.0000,0.3333,-7.0000,"Psc"},
{23.8333,24.0000,-7.0000,"Psc"},
{14.2500,14.6667,-8.0000,"Vir"},
{15.9167,16.2667,-8.0000,"Oph"},
{20.0000,20.5333,-9.0000,"Aql"},
{21.3333,21.8667,-9.0000,"Aqr"},
{17.1667,17.9667,-10.0000,"Oph"},
{5.8333,8.0833,-11.0000,"Mon"},
{4.9167,5.0833,-11.0000,"Eri"},
{5.0833,5.8333,-11.0000,"Ori"},
{8.0833,8.3667,-11.0000,"Hya"},
{9.5833,10.7500,-11.0000,"Sex"},
{11.8333,12.8333,-11.0000,"Vir"},
{17.5833,17.6667,-11.6667,"Oph"},
{18.8667,20.0000,-12.0333,"Aql"},
{4.8333,4.9167,-14.5000,"Eri"},
{20.5333,21.3333,-15.0000,"Aqr"},
{17.1667,18.2500,-16.0000,"Ser"},
{18.2500,18.8667,-16.0000,"Sct"},
{8.3667,8.5833,-17.0000,"Hya"},
{16.2667,16.3750,-18.2500,"Oph"},
{8.5833,9.0833,-19.0000,"Hya"},
{10.7500,10.8333,-19.0000,"Crt"},
{16.2667,16.3750,-19.2500,"Sco"},
{15.6667,15.9167,-20.0000,"Lib"},
{12.5833,12.8333,-22.0000,"Crv"},
{12.8333,14.2500,-22.0000,"Vir"},
{9.0833,9.7500,-24.0000,"Hya"},
{1.6667,2.6500,-24.3833,"Cet"},
{2.6500,3.7500,-24.3833,"Eri"},
{10.8333,11.8333,-24.5000,"Crt"},
{11.8333,12.5833,-24.5000,"Crv"},
{14.2500,14.9167,-24.5000,"Lib"},
{16.2667,16.7500,-24.5833,"Oph"},
{0.0000,1.6667,-25.5000,"Cet"},
{21.3333,21.8667,-25.5000,"Cap"},
{21.8667,23.8333,-25.5000,"Aqr"},
{23.8333,24.0000,-25.5000,"Cet"},
{9.7500,10.2500,-26.5000,"Hya"},
{4.7000,4.8333,-27.2500,"Eri"},
{4.8333,6.1167,-27.2500,"Lep"},
{20.0000,21.3333,-28.0000,"Cap"},
{10.2500,10.5833,-29.1667,"Hya"},
{12.5833,14.9167,-29.5000,"Hya"},
{14.9167,15.6667,-29.5000,"Lib"},
{15.6667,16.0000,-29.5000,"Sco"},
{4.5833,4.7000,-30.0000,"Eri"},
{16.7500,17.6000,-30.0000,"Oph"},
{17.6000,17.8333,-30.0000,"Sgr"},
{10.5833,10.8333,-31.1667,"Hya"},
{6.1167,7.3667,-33.0000,"CMa"},
{12.2500,12.5833,-33.0000,"Hya"},
{10.8333,12.2500,-35.0000,"Hya"},
{3.5000,3.7500,-36.0000,"For"},
{8.3667,9.3667,-36.7500,"Pyx"},
{4.2667,4.5833,-37.0000,"Eri"},
{17.8333,19.1667,-37.0000,"Sgr"},
{21.3333,23.0000,-37.0000,"PsA"},
{23.0000,23.3333,-37.0000,"Scl"},
{3.0000,3.5000,-39.5833,"For"},
{9.3667,11.0000,-39.7500,"Ant"},
{0.0000,1.6667,-40.0000,"Scl"},
{1.6667,3.0000,-40.0000,"For"},
{3.8667,4.2667,-40.0000,"Eri"},
{23.3333,24.0000,-40.0000,"Scl"},
{14.1667,14.9167,-42.0000,"Cen"},
{15.6667,16.0000,-42.0000,"Lup"},
{16.0000,16.4208,-42.0000,"Sco"},
{4.8333,5.0000,-43.0000,"Cae"},
{5.0000,6.5833,-43.0000,"Col"},
{8.0000,8.3667,-43.0000,"Pup"},
{3.4167,3.8667,-44.0000,"Eri"},
{16.4208,17.8333,-45.5000,"Sco"},
{17.8333,19.1667,-45.5000,"CrA"},
{19.1667,20.3333,-45.5000,"Sgr"},
{20.3333,21.3333,-45.5000,"Mic"},
{3.0000,3.4167,-46.0000,"Eri"},
{4.5000,4.8333,-46.5000,"Cae"},
{15.3333,15.6667,-48.0000,"Lup"},
{0.0000,2.3333,-48.1667,"Phe"},
{2.6667,3.0000,-49.0000,"Eri"},
{4.0833,4.2667,-49.0000,"Hor"},
{4.2667,4.5000,-49.0000,"Cae"},
{21.3333,22.0000,-50.0000,"Gru"},
{6.0000,8.0000,-50.7500,"Pup"},
{8.0000,8.1667,-50.7500,"Vel"},
{2.4167,2.6667,-51.0000,"Eri"},
{3.8333,4.0833,-51.0000,"Hor"},
{0.0000,1.8333,-51.5000,"Phe"},
{6.0000,6.1667,-52.5000,"Car"},
{8.1667,8.4500,-53.0000,"Vel"},
{3.5000,3.8333,-53.1667,"Hor"},
{3.8333,4.0000,-53.1667,"Dor"},
{0.0000,1.5833,-53.5000,"Phe"},
{2.1667,2.4167,-54.0000,"Eri"},
{4.5000,5.0000,-54.0000,"Pic"},
{15.0500,15.3333,-54.0000,"Lup"},
{8.4500,8.8333,-54.5000,"Vel"},
{6.1667,6.5000,-55.0000,"Car"},
{11.8333,12.8333,-55.0000,"Cen"},
{14.1667,15.0500,-55.0000,"Lup"},
{15.0500,15.3333,-55.0000,"Nor"},
{4.0000,4.3333,-56.5000,"Dor"},
{8.8333,11.0000,-56.5000,"Vel"},
{11.0000,11.2500,-56.5000,"Cen"},
{17.5000,18.0000,-57.0000,"Ara"},
{18.0000,20.3333,-57.0000,"Tel"},
{22.0000,23.3333,-57.0000,"Gru"},
{3.2000,3.5000,-57.5000,"Hor"},
{5.0000,5.5000,-57.5000,"Pic"},
{6.5000,6.8333,-58.0000,"Car"},
{0.0000,1.3333,-58.5000,"Phe"},
{1.3333,2.1667,-58.5000,"Eri"},
{23.3333,24.0000,-58.5000,"Phe"},
{4.3333,4.5833,-59.0000,"Dor"},
{15.3333,16.4208,-60.0000,"Nor"},
{20.3333,21.3333,-60.0000,"Ind"},
{5.5000,6.0000,-61.0000,"Pic"},
{15.1667,15.3333,-61.0000,"Cir"},
{16.4208,16.5833,-61.0000,"Ara"},
{14.9167,15.1667,-63.5833,"Cir"},
{16.5833,16.7500,-63.5833,"Ara"},
{6.0000,6.8333,-64.0000,"Pic"},
{6.8333,9.0333,-64.0000,"Car"},
{11.2500,11.8333,-64.0000,"Cen"},
{11.8333,12.8333,-64.0000,"Cru"},
{12.8333,14.5333,-64.0000,"Cen"},
{13.5000,13.6667,-65.0000,"Cir"},
{16.7500,16.8333,-65.0000,"Ara"},
{2.1667,3.2000,-67.5000,"Hor"},
{3.2000,4.5833,-67.5000,"Ret"},
{14.7500,14.9167,-67.5000,"Cir"},
{16.8333,17.5000,-67.5000,"Ara"},
{17.5000,18.0000,-67.5000,"Pav"},
{22.0000,23.3333,-67.5000,"Tuc"},
{4.5833,6.5833,-70.0000,"Dor"},
{13.6667,14.7500,-70.0000,"Cir"},
{14.7500,17.0000,-70.0000,"TrA"},
{0.0000,1.3333,-75.0000,"Tuc"},
{3.5000,4.5833,-75.0000,"Hyi"},
{6.5833,9.0333,-75.0000,"Vol"},
{9.0333,11.2500,-75.0000,"Car"},
{11.2500,13.6667,-75.0000,"Mus"},
{18.0000,21.3333,-75.0000,"Pav"},
{21.3333,23.3333,-75.0000,"Ind"},
{23.3333,24.0000,-75.0000,"Tuc"},
{0.7500,1.3333,-76.0000,"Tuc"},
{0.0000,3.5000,-82.5000,"Hyi"},
{7.6667,13.6667,-82.5000,"Cha"},
{13.6667,18.0000,-82.5000,"Aps"},
{3.5000,7.6667,-85.0000,"Men"},
{0.0000,24.0000,-90.0000,"Oct"}
};


map<string,ConstellationFinder::Constellation> abbrev_table
{
{"And", {"And","Andromeda","Andromedae"}},
{"Ant", {"Ant","Antlia","Antliae"}},
{"Aps", {"Aps","Apus","Apodis"}},
{"Aql", {"Aql","Aquila","Aquilae"}},
{"Aqr", {"Aqr","Aquarius","Aquarii"}},
{"Ara", {"Ara","Ara","Arae"}},
{"Ari", {"Ari","Aries","Arietis"}},
{"Aur", {"Aur","Auriga","Aurigae"}},
{"Boo", {"Boo","Bo\u00f6tes","Bo\u00f6tis"}},
{"CMa", {"CMa","Canis Major","Canis Majoris"}},
{"CMi", {"CMi","Canis Minor","Canis Minoris"}},
{"CVn", {"CVn","Canes Venatici","Canum Venaticorum"}},
{"Cae", {"Cae","Caelum","Caeli"}},
{"Cam", {"Cam","Camelopardalis","Camelopardalis"}},
{"Cap", {"Cap","Capricornus","Capricorni"}},
{"Car", {"Car","Carina","Carinae"}},
{"Cas", {"Cas","Cassiopeia","Cassiopeiae"}},
{"Cen", {"Cen","Centaurus","Centauri"}},
{"Cep", {"Cep","Cepheus","Cephei"}},
{"Cet", {"Cet","Cetus","Ceti"}},
{"Cha", {"Cha","Chamaeleon","Chamaeleontis"}},
{"Cir", {"Cir","Circinus","Circini"}},
{"Cnc", {"Cnc","Cancer","Cancri"}},
{"Col", {"Col","Columba","Columbae"}},
{"Com", {"Com","Coma Berenices","Comae Berenices"}},
{"CrA", {"CrA","Corona Australis","Coronae Australis"}},
{"CrB", {"CrB","Corona Borealis","Coronae Borealis"}},
{"Crt", {"Crt","Crater","Crateris"}},
{"Cru", {"Cru","Crux","Crucis"}},
{"Crv", {"Crv","Corvus","Corvi"}},
{"Cyg", {"Cyg","Cygnus","Cygni"}},
{"Del", {"Del","Delphinus","Delphini"}},
{"Dor", {"Dor","Dorado","Doradus"}},
{"Dra", {"Dra","Draco","Draconis"}},
{"Equ", {"Equ","Equuleus","Equulei"}},
{"Eri", {"Eri","Eridanus","Eridani"}},
{"For", {"For","Fornax","Fornacis"}},
{"Gem", {"Gem","Gemini","Geminorum"}},
{"Gru", {"Gru","Grus","Gruis"}},
{"Her", {"Her","Hercules","Herculis"}},
{"Hor", {"Hor","Horologium","Horologii"}},
{"Hya", {"Hya","Hydra","Hydrae"}},
{"Hyi", {"Hyi","Hydrus","Hydri"}},
{"Ind", {"Ind","Indus","Indi"}},
{"LMi", {"LMi","Leo Minor","Leonis Minoris"}},
{"Lac", {"Lac","Lacerta","Lacertae"}},
{"Leo", {"Leo","Leo","Leonis"}},
{"Lep", {"Lep","Lepus","Leporis"}},
{"Lib", {"Lib","Libra","Librae"}},
{"Lup", {"Lup","Lupus","Lupi"}},
{"Lyn", {"Lyn","Lynx","Lyncis"}},
{"Lyr", {"Lyr","Lyra","Lyrae"}},
{"Men", {"Men","Mensa","Mensae"}},
{"Mic", {"Mic","Microscopium","Microscopii"}},
{"Mon", {"Mon","Monoceros","Monocerotis"}},
{"Mus", {"Mus","Musca","Muscae"}},
{"Nor", {"Nor","Norma","Normae"}},
{"Oct", {"Oct","Octans","Octantis"}},
{"Oph", {"Oph","Ophiuchus","Ophiuchi"}},
{"Ori", {"Ori","Orion","Orionis"}},
{"Pav", {"Pav","Pavo","Pavonis"}},
{"Peg", {"Peg","Pegasus","Pegasi"}},
{"Per", {"Per","Perseus","Persei"}},
{"Phe", {"Phe","Phoenix","Phoenicis"}},
{"Pic", {"Pic","Pictor","Pictoris"}},
{"PsA", {"PsA","Piscis Austrinus","Piscis Austrini"}},
{"Psc", {"Psc","Pisces","Piscium"}},
{"Pup", {"Pup","Puppis","Puppis"}},
{"Pyx", {"Pyx","Pyxis","Pyxidis"}},
{"Ret", {"Ret","Reticulum","Reticuli"}},
{"Scl", {"Scl","Sculptor","Sculptoris"}},
{"Sco", {"Sco","Scorpius","Scorpii"}},
{"Sct", {"Sct","Scutum","Scuti"}},
{"Ser", {"Ser","Serpens","Serpentis"}},
{"Sex", {"Sex","Sextans","Sextantis"}},
{"Sge", {"Sge","Sagitta","Sagittae"}},
{"Sgr", {"Sgr","Sagittarius","Sagittarii"}},
{"Tau", {"Tau","Taurus","Tauri"}},
{"Tel", {"Tel","Telescopium","Telescopii"}},
{"TrA", {"TrA","Triangulum Australe","Trianguli Australis"}},
{"Tri", {"Tri","Triangulum","Trianguli"}},
{"Tuc", {"Tuc","Tucana","Tucanae"}},
{"UMa", {"UMa","Ursa Major","Ursae Majoris"}},
{"UMi", {"UMi","Ursa Minor","Ursae Minoris"}},
{"Vel", {"Vel","Vela","Velorum"}},
{"Vir", {"Vir","Virgo","Virginis"}},
{"Vol", {"Vol","Volans","Volantis"}},
{"Vul", {"Vul","Vulpecula","Vulpeculae"}}
};


pair<double,double> precess(double ra1, double dec1, double epoch1, double epoch2) {
  struct _3Coords {
    double _0;
    double _1;
    double _2;
  };
  double cdr, csr;
  vector<vector<double>> r{vector<double>{0.0, 0.0, 0.0}, vector<double>{0.0, 0.0, 0.0}, vector<double>{0.0, 0.0, 0.0}};
  
  double t, st, a, b, c, sina, sinb, sinc, cosa, cosb, cosc, ra2, dec2;

  cdr = M_PI / 180.0;
  csr = cdr / 3600.0;
  a = cos(dec1);
  vector<double> x1{a* cos(ra1), a* sin(ra1), sin(dec1)};
  t = 0.001*(epoch2 - epoch1);
  st = 0.001*(epoch1 - 1900.0);
  a = csr*t*(23042.53 + st*(139.75 + 0.06*st) + t*(30.23 - 0.27*st + 18.0*t));
  b = csr*t*t*(79.27 + 0.66*st + 0.32*t) + a;
  c = csr*t*(20046.85 - st*(85.33 + 0.37*st) + t*(-42.67 - 0.37*st - 41.8*t));
  sina = sin(a);
  sinb = sin(b);
  sinc = sin(c);
  cosa = cos(a);
  cosb = cos(b);
  cosc = cos(c);
  r[0][0] = cosa*cosb*cosc - sina*sinb;
  r[0][1] = -cosa*sinb - sina*cosb*cosc;
  r[0][2] = -cosb*sinc;
  r[1][0] = sina*cosb + cosa*sinb*cosc;
  r[1][1] = cosa*cosb - sina*sinb*cosc;
  r[1][2] = -sinb*sinc;
  r[2][0] = cosa*sinc;
  r[2][1] = -sina*sinc;
  r[2][2] = cosc;
  vector<double> x2 {0.0, 0.0, 0.0};
  for (int i = 0; i < 3; i++)
      x2[i] = r[i][0]*x1[0] + r[i][1]*x1[1] + r[i][2]*x1[2];
  ra2 = atan2(x2[1], x2[0]);
  if (ra2 < 0.0)
     ra2 += 2.0 * M_PI;
  dec2 = asin(x2[2]);
  return {ra2, dec2};
}

}
ConstellationFinder::Constellation ConstellationFinder::getName(const Coordinates::Equatorial &coordinates, double epoch)
{
  return getName(coordinates.rightAscension.hours(), coordinates.declination.degrees(), epoch);
}
ConstellationFinder::Constellation ConstellationFinder::getName(double ra, double dec, double epoch)
{
  double convh, convd;
    convh = M_PI / 12.0;
  convd = M_PI / 180.0;
  ra *= convh;
  dec *= convd;
  pair<double,double> newcoords = precess(ra, dec, epoch, 1875.0);
  ra = newcoords.first;
  dec = newcoords.second;
  ra /= convh;
  dec /= convd;
  for (int i = 0; i < coordinatesTable.size(); i++)
  {
    if (dec < coordinatesTable[i].lDec || ra < coordinatesTable[i].lAR || ra >= coordinatesTable[i].uAR)
       continue;
    return abbrev_table[coordinatesTable[i].abbr];
  }
  return {};    // Error!
}

vector< ConstellationFinder::Constellation > ConstellationFinder::constellations()
{
  vector<Constellation> out;
  std::transform(begin(abbrev_table), end(abbrev_table), back_inserter(out), [](pair<string,Constellation> p){ return p.second; });
  return out;
}
  
ConstellationFinder::Constellation ConstellationFinder::byAbbrev(const string &abbrev)
{
  return abbrev_table.count(abbrev) > 0 ? abbrev_table[abbrev] : Constellation{};
}

