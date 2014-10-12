#define BOOST_TEST_MAIN
#include "test_helper.h"

#include "webservice/object.h"
#include <Wt/Json/Object>
#include <Wt/Json/Serializer>
#include <Wt/Json/Parser>

using namespace std;

class AnObject : public WtCommons::Json::Object {
public:
    AnObject() {
        addField<string>("string", _string).addField<int>("number", _number);
    }
    AnObject(int a_number, const string &a_string) : _number(a_number), _string(a_string) {
        addField<string>("string", _string).addField<int>("number", _number);
    }
    std::string _string;
    int _number;
};

BOOST_AUTO_TEST_CASE(TestConstruction) {
    AnObject anObject(5, "3");

    Wt::Json::Object o;
    Wt::Json::parse(R"({"number" : 5, "string" : "3" })", o);

    BOOST_REQUIRE_EQUAL(Wt::Json::serialize(o), anObject.toJson());
}

BOOST_AUTO_TEST_CASE(Parsing) {
    AnObject anObject;

    anObject.fromJson(R"({"number" : 5, "string" : "3" })");

    BOOST_REQUIRE_EQUAL(5, anObject._number);
    BOOST_REQUIRE_EQUAL("3", anObject._string);
}

