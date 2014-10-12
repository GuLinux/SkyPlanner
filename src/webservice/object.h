#ifndef WTCOMMONS_JSON_OBJECT_H
#define WTCOMMONS_JSON_OBJECT_H

#include <memory>
#include <map>
#include <boost/date_time.hpp>

namespace Wt {
namespace Json {
class Value;
class Object;
}
}

namespace WtCommons {
namespace Json {

template<typename T> class FieldBuilder;
class Object
{
    struct Field {
        void *p;
        enum Type {String, Int, LongLong, DateTime} type;
        template<typename T> struct Builder {
            static Field build(T &t);
        };
    };

public:
    template<typename T>
    Object &addField(const std::string &name, T &field);

    std::string toJson() const;
    void fromJson(const std::string &jsonString);
private:
    std::map<std::string, Field> fields;
    template<typename T> friend class FieldBuilder;
    void from(const Wt::Json::Object &);
};

template<typename T>
Object &Object::addField(const std::string &name, T &field) {
    fields[name] = Field::Builder<T>::asField(field);
    return *this;
}


template<> class Object::Field::Builder<std::string> {
public:
    static Object::Field asField(std::string &s) { return {&s, Object::Field::String}; }
};
template<> class Object::Field::Builder<int> {
public:
    static Object::Field asField(int &i) { return {&i, Object::Field::Int}; }
};
template<> class Object::Field::Builder<long long> {
public:
    static Object::Field asField(long long &i) { return {&i, Object::Field::LongLong}; }
};
template<> class Object::Field::Builder<boost::posix_time::ptime> {
public:
    static Object::Field asField(boost::posix_time::ptime &v) { return {&v, Object::Field::DateTime}; }
};


} // namespace Json
} // namespace WtCommons

#endif // WTCOMMONS_JSON_OBJECT_H
