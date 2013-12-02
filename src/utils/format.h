#ifndef GUM_FORMAT_H
#define GUM_FORMAT_H
#include <boost/format.hpp>
#include <Wt/WString>

class format {
public:
  format(const std::string &formatString) : _format(formatString) {}
  operator std::string() { return str(); }
  operator Wt::WString() { return wtstr(); }
  template<typename FormatValue>
  format &operator%(FormatValue formatValue) { _format % formatValue;  return *this; }
  std::string str() const { return _format.str(); }
  Wt::WString wtstr() const { return Wt::WString::fromUTF8(_format.str()); }
private:
  boost::format _format;
};

#endif