#define BOOST_TEST_MAIN
#include "test_helper.h"

#include "webservice/object.h"

using namespace std;

class AnObject : public WtCommons::Json::Object {
public:
    AnObject(int a_number, const string &a_string) : _number(a_number), _string(a_string) {
        addField<string>("string", _string).addField<int>("number", _number);
    }
    std::string _string;
    int _number;
};

BOOST_AUTO_TEST_CASE(TestConstruction) {
    AnObject anObject(5, "3");

    Wt::Json::Object o;

    BOOST_REQUIRE_EQUAL(R"(
{
    "number" : 5,
    "string" : "3"
})", anObject.toJson());
}

