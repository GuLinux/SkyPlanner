#include <boost/python.hpp>
#include "ephemeris.hpp"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


using namespace boost::python;


BOOST_PYTHON_MODULE(ephemeris) {
    typedef std::vector<RiseTransitSet> RiseTransitSetList;
    class_<Coordinates>("Coordinates").def_readwrite("ra", &Coordinates::ra).def_readwrite("dec", &Coordinates::dec);
    def("coordinates", coordinates);
    RiseTransitSet(Ephemeris::*rst_single_coordinates)(const Coordinates &, double, double) const = &Ephemeris::rst;
    RiseTransitSetList(Ephemeris::*rst_vector_coordinates)(const boost::python::list &, double, double) const = &Ephemeris::rst;
    class_<Ephemeris>("Ephemeris", init<double, double>())
        .def("rst", rst_single_coordinates)
        .def("rst_list", rst_vector_coordinates)
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
