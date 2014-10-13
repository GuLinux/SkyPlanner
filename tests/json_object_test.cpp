#define BOOST_TEST_MAIN
#include "test_helper.h"

#include "Wt-Commons/json_object.h"

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

template<typename T>
class AnObjectWithAnArray : public WtCommons::Json::Object {
public:
    AnObjectWithAnArray() {
        addField<std::vector<T>>("an-array", anArray).addField<int>("number", _number);
    }
    std::vector<T> anArray;
    int _number;
};

class AnotherObject : public WtCommons::Json::Object {
public:
    AnotherObject() {
        addField<string>("another_string", _string).addField<int>("another_number", _number).addField<WtCommons::Json::Object>("an_object", _anObject);
    }
    AnotherObject(int a_number, const string &a_string, const AnObject &anObject) : _number(a_number), _string(a_string), _anObject(anObject) {
        addField<string>("another_string", _string).addField<int>("another_number", _number).addField<WtCommons::Json::Object>("an_object", _anObject);
    }
    std::string _string;
    int _number;
    AnObject _anObject;
};

BOOST_AUTO_TEST_CASE(TestConstruction) {
    AnObject anObject(5, "3");

    Wt::Json::Object o;
    Wt::Json::parse(R"({"number" : 5, "string" : "3" })", o);

    BOOST_REQUIRE_EQUAL(Wt::Json::serialize(o), anObject.toJson());
}
BOOST_AUTO_TEST_CASE(TestConstructionWithArray) {
    AnObjectWithAnArray<int> anObject;
    anObject._number = 4;
    anObject.anArray.push_back(5);
    anObject.anArray.push_back(6);
    anObject.anArray.push_back(9);

    Wt::Json::Object o;
    Wt::Json::parse(R"({"number" : 4, "an-array" : [5, 6, 9] })", o);

    BOOST_REQUIRE_EQUAL(Wt::Json::serialize(o), anObject.toJson());
}


BOOST_AUTO_TEST_CASE(TestNestedObjectConstruction) {
    AnObject anObject(5, "3");
    AnotherObject anotherObject(233, "123", anObject);

    Wt::Json::Object o;
    Wt::Json::parse(R"({
                    "an_object" : {
                        "number" : 5,
                        "string" : "3"
                    },
                    "another_number" : 233,
                    "another_string" : "123"
                })", o);

    BOOST_REQUIRE_EQUAL(Wt::Json::serialize(o), anotherObject.toJson());
}

BOOST_AUTO_TEST_CASE(Parsing) {
    AnObject anObject;

    anObject.fromJson(R"({"number" : 5, "string" : "3" })");

    BOOST_REQUIRE_EQUAL(5, anObject._number);
    BOOST_REQUIRE_EQUAL("3", anObject._string);
}

BOOST_AUTO_TEST_CASE(ParsingNestes) {
    AnotherObject anObject;

    anObject.fromJson(R"({
                      "an_object" : {
                          "number" : 5,
                          "string" : "3"
                      },
                      "another_number" : 233,
                      "another_string" : "123"
                  })");

    BOOST_REQUIRE_EQUAL(233, anObject._number);
    BOOST_REQUIRE_EQUAL("123", anObject._string);
    BOOST_REQUIRE_EQUAL("3", anObject._anObject._string);
    BOOST_REQUIRE_EQUAL(5, anObject._anObject._number);
}

