#include <boost/python.hpp>
#include "ephemeris.hpp"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <vector>

using namespace boost::python;
using namespace std;

template<typename T> void list2vector(const boost::python::list &l, vector<T> &v) {
    for(int i = 0; i < len(l); i++) {
        v[i] = boost::python::extract<T>(l[i]);
    }
}
#define CONVERT_LIST(source, variable, type) vector<type> variable(len(source)); list2vector(source, variable);

RiseTransitSet Ephemeris_rst(Ephemeris &e, const Coordinates &coordinates, double jd, double horizon) {
    return e.rst(coordinates, jd, horizon);
}
vector<RiseTransitSet> Ephemeris_rst_list(Ephemeris &e, const boost::python::list &coordinates, double jd, double horizon) {
    CONVERT_LIST(coordinates, v, Coordinates)
    return e.rst(v, jd, horizon);
}

vector<RiseTransitSet> Ephemeris_rst_parallel(Ephemeris &e, const boost::python::list &coordinates, double jd, double horizon) {
    CONVERT_LIST(coordinates, v, Coordinates)
    return e.rst_parallel(v, jd, horizon);
}



BOOST_PYTHON_MODULE(ephemeris) {
    typedef vector<RiseTransitSet> RiseTransitSetList;
    class_<Coordinates>("Coordinates").def_readwrite("ra", &Coordinates::ra).def_readwrite("dec", &Coordinates::dec);
    def("coordinates", coordinates);
    class_<Ephemeris>("Ephemeris", init<double, double>())
        .def("rst", Ephemeris_rst)
        .def("rst_list", Ephemeris_rst_list)
        .def("rst_parallel", Ephemeris_rst_parallel)
    ;

    {
        scope rst = class_<RiseTransitSet>("RiseTransitSet")
            .def_readonly("rise", &RiseTransitSet::rise)
            .def_readonly("transit", &RiseTransitSet::transit)
            .def_readonly("set", &RiseTransitSet::set)
            .def_readonly("type", &RiseTransitSet::type)
        ;
        enum_<RiseTransitSet::Type>("Type")
            .value("CircumPolar", RiseTransitSet::CircumPolar)
            .value("Rises", RiseTransitSet::Rises)
            .value("NeverRises", RiseTransitSet::NeverRises)
        ;

        class_<RiseTransitSetList>("List")
            .def(vector_indexing_suite<RiseTransitSetList>() );

    }
    
}
