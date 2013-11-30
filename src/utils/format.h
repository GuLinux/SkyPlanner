#ifndef GUM_FORMAT_H
#define GUM_FORMAT_H
#include <boost/format.hpp>
#include <Wt/WString>

class format {
public:
  format(const std::string &formatString) : _format(formatString) {}
  operator std::string() { return _format.str(); }
  operator Wt::WString() { return Wt::WString::fromUTF8(_format.str()); }
  template<typename FormatValue>
  format &operator%(FormatValue formatValue) { _format % formatValue;  return *this; }
private:
  boost::format _format;
};

#endif