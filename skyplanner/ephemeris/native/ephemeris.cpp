#include "ephemeris.hpp"
#include <algorithm>
#include <map>
using namespace std;

Ephemeris::Ephemeris(double lat, double lng) : observer{lng, lat}
{
}

Ephemeris::~Ephemeris() {
}


RiseTransitSet Ephemeris::rst(const Coordinates &coordinates, double jd, double horizon) const
{
    static map<int, RiseTransitSet::Type> types {
        {0, RiseTransitSet::Rises}, {-1, RiseTransitSet::NeverRises}, {1, RiseTransitSet::CircumPolar},
    };
    ln_equ_posn coords{coordinates.ra, coordinates.dec};
    ln_rst_time rst_time;
    int result = ln_get_object_rst_horizon(jd, const_cast<ln_lnlat_posn*>(&observer), &coords, horizon, &rst_time); 
    return {rst_time.rise, rst_time.transit, rst_time.set, types[result]}; 
}


vector<RiseTransitSet> Ephemeris::rst(const boost::python::list &coordinates, double jd, double horizon) const
{
    vector<Coordinates> coordinates_vector(len(coordinates));
    for(size_t i = 0; i < len(coordinates); i++) {
        coordinates_vector[i] = boost::python::extract<Coordinates>(coordinates[i]);
    }
    return rst(coordinates_vector, jd, horizon);
}

vector<RiseTransitSet> Ephemeris::rst(const vector<Coordinates> &coordinates, double jd, double horizon) const
{
    vector<RiseTransitSet> result(coordinates.size());
    transform(begin(coordinates), end(coordinates), begin(result), [&](const Coordinates &c) { return rst(c, jd, horizon); }); 
    return result;
}


bool RiseTransitSet::operator==(const RiseTransitSet &other) const {
    return other.rise == rise && 
           other.set == set &&
           other.transit == transit &&
           other.type == type;       
}
bool RiseTransitSet::operator!=(const RiseTransitSet &other) const {
    return ! (*this == other);
}

