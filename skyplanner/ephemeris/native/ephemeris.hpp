#ifndef SKYPLANNER_EPHEMERIS_H
#define SKYPLANNER_EPHEMERIS_H

#include <memory>
#include <libnova/libnova.h>
#include <vector>
#include <boost/python.hpp>

typedef ln_equ_posn Coordinates;
inline Coordinates coordinates(double ra, double dec) { return Coordinates{ra, dec}; }

struct RiseTransitSet {
    double rise, transit, set;
    enum Type { CircumPolar, Rises, NeverRises };
    Type type;
	bool operator==(const RiseTransitSet &other) const;
	bool operator!=(const RiseTransitSet &other) const;
};

class Ephemeris {
    public:
        Ephemeris(double lat, double lng);
        ~Ephemeris();
        RiseTransitSet rst(const Coordinates &coordinates, double jd, double horizon) const;
        std::vector<RiseTransitSet> rst(const std::vector<Coordinates> &coordinates, double jd, double horizon) const;
        std::vector<RiseTransitSet> rst(const boost::python::list &coordinates, double jd, double horizon) const;
    private:
        ln_lnlat_posn observer;
};

#endif
