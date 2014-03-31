#ifndef DSS_NAMESPACE
#define DSS_NAMESPACE
#include <map>
#include <string>
#include <vector>

namespace DSS {
    enum ImageVersion {
      poss2ukstu_red,
      poss2ukstu_blue,
      poss2ukstu_ir,
      poss1_red,
      poss1_blue,
      quickv,
      phase2_gsc2,
      phase2_gsc1,
    };

    DSS::ImageVersion imageVersion(const std::string &version);
    std::string imageVersion(const DSS::ImageVersion &version);
    std::vector<DSS::ImageVersion> versions();

}

#endif

