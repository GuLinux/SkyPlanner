/*
 * Copyright (C) 2015  <copyright holder> <email>
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
 */

#include "urls.h"
#include <boost/format.hpp>

using namespace std;

const string URLs::home = {"/home/"};
const string URLs::loading_indicator = {"/loading-64.png"};
const string URLs::skyplanner_logo = {"/skyplanner_logo.png"};
const string URLs::bootstrap_url = {"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css"};
const string URLs::moon_phase_image(double phase)
{
  int phase_index = int( 36.0*phase+0.5 ) % 36;
//   cerr << "phase: input=" << fixed << setprecision(2) << phase << ", output=" << phase_index << endl;
  return (boost::format("/moon_phases/moon%02d.png") % phase_index).str();
}
