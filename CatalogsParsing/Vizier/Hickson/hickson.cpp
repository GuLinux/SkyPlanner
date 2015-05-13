#include <iostream>
#include "vizier.hpp"
#include "session.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

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
    string name{"Hickson's Compact groups of Galaxies"};
    Vizier vizier{vm};

    Dbo::Transaction t(vizier.session() );
    CataloguePtr catalogue = vizier.findOrCreateCatalogue({
                                                            [=](Dbo::Query<CataloguePtr> &query){ query.where("name = ?").bind(name); },
                                                            [=](){ return new Catalogue{name, "B", -90, Catalogue::ByCatalog, Catalogue::Visible}; },
                                                          }, t);
    map<string,NebulaDenominationPtr> denominations;
    for(auto row: vizier.rows()) {
      if(! denominations[row["HCG"].value]) {
	
	NgcObjectPtr object;
	
	vector<string> otherCatalogues;
	for(int i=1; i<5; i++) {
	  auto catCol = (boost::format("Aname%d") % i).str();
	  std::cerr << "other catalogues for HCG " << row["HCG"].value << "-" << catCol << ": " << row[catCol].value << std::endl;
	  
	  string cat = row[catCol].value;
	  if(cat.empty())
	    continue;
	  std::string::const_iterator start, end;
	  start = cat.begin();
	  end = cat.end();
	  boost::regex re("([A-Z]+)([0-9]+)");
	  boost::match_results<std::string::const_iterator> what;
	  if(!boost::regex_search(start, end, what, re))
	    continue;
	  static map<string,string> cats { {"N", "NGC"}, {"U", "UGC"}, {"I", "IC"}, {"ARP", "ARP"} };
	  string catName = what[1];
	  string catNum = what[2];
	  if(cats.count(catName) == 0)
	    continue;
	  otherCatalogues.push_back((boost::format("%s %s") % cats[catName] % catNum).str() );
	  if(!object) {
	    try {
	      CataloguePtr cat = t.session().find<Catalogue>().where("code = ?").bind(cats[catName]);
	      if(!cat) continue;
	      std::cerr << "found cat: " << cat->name() << std::endl;
	      NebulaDenominationPtr den = t.session().find<NebulaDenomination>().where("catalogues_id = ? AND number = ?").bind(cat.id()).bind(catNum);
	      if(!den) continue;
	      object= den->ngcObject();
	      if(object) std::cout << "found match HCG " << row["HCG"].value << "==: " << *object->objectId() << std::endl;
	    } catch(std::exception &) {}
	  }
	}
	
        Angle ar = row["_RAJ2000"].degrees();
        Angle dec = row["_DEJ2000"].degrees();
        Angle angularSize = row["AngSize"].arcMinutes();
	double mag = row["TotMag"].asDouble();
        string objectName(string{"HCG "} + row["HCG"].value);
        cout << "Adding ngc object: " << objectName << ", AR: " << ar.printable(Angle::Hourly) << ", DEC=" << dec.printable() << ", angular size: " << angularSize.printable() << endl;
	if(!object) {
	  cout << "Object not found for catalogues " << boost::join(otherCatalogues, "-") << ", creating new" << std::endl;
	  object = vizier.session().add(new NgcObject{ar, dec, mag, angularSize, NgcObject::NebGalGroups, objectName});
	}
        boost::optional<string> comment;
        vector<string> comments;
        for(auto r: vizier.rows() ) {
          if(r["HCG"].value == row["HCG"].value && find(begin(comments), end(comments), r["Comments"].value) == end(comments) ) {
            comments.push_back(r["Comments"].value);
          }
        }
        if( comments.size() )
          comment.reset(boost::algorithm::join(comments, " "));

	auto otherCatalogue = boost::optional<string>{};
	if(!otherCatalogues.empty())
	  otherCatalogue.reset(boost::join(otherCatalogues, "; "));
        string name = string{"HCG "} + row["HCG"].value;
        auto number = boost::optional<string>{row["HCG"].value};
        NebulaDenominationPtr denomination = vizier.session().add(new NebulaDenomination(
                                                                    catalogue,
                                                                    number,
                                                                    name,
                                                                    comment,
                                                                    otherCatalogue
                                                                  ));
        object.modify()->nebulae().insert(denomination);
        denominations[row["HCG"].value] = denomination;
      }
    }
  } catch(exception &e) {
    cerr << e.what() << endl << options << endl;
    return 1;
  }

  return 0;
}
