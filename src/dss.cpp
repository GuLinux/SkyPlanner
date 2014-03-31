#include "dss.h"
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
using namespace DSS;


static std::map<ImageVersion,std::string> imageVersionStrings{
  { poss2ukstu_red, "poss2ukstu_red" },
  { poss2ukstu_blue, "poss2ukstu_blue" },
  { poss2ukstu_ir, "poss2ukstu_ir" },
  { poss1_red, "poss1_red" },
  { poss1_blue, "poss1_blue" },
  { quickv, "quickv" },
  { phase2_gsc2, "phase2_gsc2" },
  { phase2_gsc1, "phase2_gsc1" },
};

ImageVersion DSS::imageVersion( const string &version )
{
  for(auto v: imageVersionStrings) {
    if(version == v.second)
      return v.first;
  }
  return phase2_gsc2;
}

string DSS::imageVersion( const ImageVersion &version )
{
  return imageVersionStrings[version];
}

vector< ImageVersion > DSS::versions()
{
  vector<ImageVersion> v;
  transform(begin(imageVersionStrings), end(imageVersionStrings), back_inserter(v), [](pair<ImageVersion,string> p) { return p.first; });
  return v;
}


