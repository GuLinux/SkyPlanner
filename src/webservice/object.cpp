#include "object.h"
#include <Wt/Json/Object>
#include <Wt/Json/Serializer>

using namespace std;

namespace WtCommons {
namespace Json {
template<typename T> class Value {
public:
    Value(void *p) : v(*reinterpret_cast<T*>(p)) {}
    operator T() const { return v;}
private:
    T v;
};

string Object::toJson() const {
    Wt::Json::Object wtObject;
    for(auto v: fields) {
        Wt::Json::Value value;
        switch(v.second.type) {
            case Field::String:
                //value = {*reinterpret_cast<string*>(v.second.p)};
                value = {Wt::WString::fromUTF8(Value<string>(v.second.p))};
                break;
            case Field::Int:
                value = {Value<int>(v.second.p)};
                break;
            case Field::LongLong:
                value = {Value<long long>(v.second.p)};
                break;
            case Field::DateTime: // TODO: Parameters for format choosing
                auto t = Value<boost::posix_time::ptime>(v.second.p);
                value = {boost::posix_time::to_iso_string(t)};
                break;
        }

        wtObject[v.first] = value;
    }
    return Wt::Json::serialize(wtObject);
}


} // namespace Json
} // namespace WtCommons
