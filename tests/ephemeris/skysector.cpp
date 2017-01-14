#include <libnova/libnova.h>
#include <functional>
#include <vector>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;
class SkySector {
    public:
        static const int STEP = 5;
        typedef pair<double, double> Range;
        static vector<SkySector> &all();
        SkySector(const Range &ra, const Range &dec, int index);
        const Range ra, dec;
        int index;
        ln_equ_posn center;
};

SkySector::SkySector(const Range &ra, const Range &dec, int index): ra{ra}, dec{dec}, index{index} {
    center.ra = ra.first + (ra.second-ra.first)/2;
    center.dec = dec.first + (dec.second-dec.first)/2;
}

vector<SkySector> &SkySector::all() {
    static vector<SkySector> _all;
    if(_all.empty()) {
        int index = 0;
        for(int dec = -90; dec < 90; dec += STEP) {
            Range dec_range{dec, dec + STEP};
            for(int ra = 0; ra < 360; ra += STEP) {
                Range ra_range{ra, ra + STEP};
                _all.push_back({ra_range, dec_range, index++});
            }
        }
    }
    return _all;
}


int main() {
    auto all = SkySector::all();
    ln_lnlat_posn observer{-0.127758, 51.507351};
    double jd= ln_get_julian_from_sys();
    vector<ln_rst_time> ephemeris;
    auto start = steady_clock::now();
    for(auto sector: all) {
        ln_rst_time rts;
        int result = ln_get_object_rst(jd, &observer, &sector.center, &rts);
        if(result >= 0) 
            ephemeris.push_back(rts);
    }
    duration<double> elapsed = steady_clock::now() - start;
    cout << "elapsed: " << elapsed.count() * 1000.<< "ms, objects size: " << ephemeris.size() << endl;
    return 0;
}
