#include <iostream>
#include "vizier.hpp"
#include "session.h"
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Wt;
namespace po=boost::program_options;

int main(int argc, char **argv)
{
  po::variables_map vm;
  auto options = Vizier::vizierProgramOptions();
  po::store(po::parse_command_line(argc, argv, options), vm);
  po::notify(vm);
  if(vm.count("help")) {
    cout << options << endl;
    return 0;
  }

  try {
    string name{"Barnard's Catalogue of Dark Objects"};
    Vizier vizier{vm};

    Dbo::Transaction t(vizier.session() );
    CataloguePtr catalogue = vizier.findOrCreateCatalogue({
                                                            [=](Dbo::Query<CataloguePtr> &query){ query.where("name = ?").bind(name); },
                                                            [=](){ return new Catalogue{name, "B", -90, Catalogue::ByCatalog, Catalogue::Visible}; },
                                                          }, t);
    map<string,NebulaDenominationPtr> denominations;
    for(auto row: vizier.rows()) {
      if(! denominations[row["Barn"].value]) {
        Angle ar = row["_RAJ2000"].degrees();
        Angle dec = row["_DEJ2000"].degrees();
        Angle angularSize = row["Diam"].arcMinutes();
        string objectName(string{"B "} + row["Barn"].value);
        cout << "Adding ngc object: " << objectName << ", AR: " << ar.printable(Angle::Hourly) << ", DEC=" << dec.printable() << ", angular size: " << angularSize.printable() << endl;
        NgcObjectPtr object = vizier.session().add(new NgcObject{ar, dec, 99, angularSize, NgcObject::NebDn, objectName});
        boost::optional<string> comment;
        vector<string> comments;
        for(auto r: vizier.rows() ) {
          if(r["Barn"].value == row["Barn"].value) {
            comments.push_back(r["Text"].value);
          }
        }
        if( comments.size() )
          comment.reset(boost::algorithm::join(comments, " "));
        auto otherCatalogue = boost::optional<string>{};
        string name = string{"B "} + row["Barn"].value;
        auto number = boost::optional<string>{row["Barn"].value};
        NebulaDenominationPtr denomination = vizier.session().add(new NebulaDenomination(
                                                                    catalogue,
                                                                    number,
                                                                    name,
                                                                    comment,
                                                                    otherCatalogue
                                                                  ));
        object.modify()->nebulae().insert(denomination);
        denominations[row["Barn"].value] = denomination;
      }
    }
  } catch(exception &e) {
    cerr << e.what() << endl << options << endl;
    return 1;
  }

  return 0;
}
