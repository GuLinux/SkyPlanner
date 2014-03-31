#include "models/Models"
#include <Wt/Dbo/Transaction>
#include <Wt/Dbo/Session>
using namespace Wt;
using namespace std;

ViewPort::ViewPort(const Coordinates::Equatorial &coordinates, const Angle &angle, DSSImage::ImageVersion imageVersion, const NgcObjectPtr &ngcObject, const UserPtr &user)
  : arcminutes(angle.arcMinutes()), ar(coordinates.rightAscension.degrees()), dec(coordinates.declination.degrees()), _imageVersion(imageVersion), _ngcObject(ngcObject), _user(user)
{
}

Angle ViewPort::angularSize() const
{
   return Angle::arcMinutes(arcminutes);
}
 
 
Coordinates::Equatorial ViewPort::coordinates() const
{
  return {Angle::degrees(ar), Angle::degrees(dec) };
}

void ViewPort::setImageVersion( DSSImage::ImageVersion imageVersion, const Dbo::ptr< NgcObject > &ngcObject, const Dbo::ptr< User > &user, Dbo::Transaction &transaction )
{
  if(!user) return;
  transaction.session().execute("UPDATE objects_viewport SET image_version = ? WHERE user_id = ? AND objects_id = ?").bind(imageVersion).bind(user.id()).bind(ngcObject.id());
}
ViewPort ViewPort::findOrCreate( DSSImage::ImageVersion imageVersion, const Dbo::ptr< NgcObject > &ngcObject, const Dbo::ptr< User > &user, Dbo::Transaction &transaction )
{
  Angle size = Angle::degrees(ngcObject->angularSize());
  double multiplyFactor = 2.0;
  if(size < Angle::arcMinutes(20))
    multiplyFactor = 3.5;
  if(size < Angle::arcMinutes(10))
    multiplyFactor = 4.5;
  if(size < Angle::arcMinutes(5) )
    multiplyFactor = 7.;
  if(size < Angle::arcMinutes(1) )
    multiplyFactor = 90.;
  size = min(Angle::arcMinutes(75.0), size * multiplyFactor);
  size = (size <= Angle::degrees(0)) ? Angle::arcMinutes(75.0) : size; // objects without angular size (-1), showing max possible field...
  
  if(!user)
    return ViewPort{ngcObject->coordinates(), size, imageVersion, ngcObject, user};
  ViewPortPtr viewPort = transaction.session().find<ViewPort>().where("objects_id = ?").bind(ngcObject.id()).where("user_id = ?").bind(user.id());
  if(!viewPort) {
    viewPort = transaction.session().add(new ViewPort{ngcObject->coordinates(), size, imageVersion, ngcObject, user});
  }
  return *viewPort;
  
}

void ViewPort::save(const Coordinates::Equatorial &coordinates, const Angle &angularSize, DSSImage::ImageVersion imageVersion, const NgcObjectPtr &ngcObject, const UserPtr &user, Dbo::Transaction &transaction)
{
  if(!user)
    return;
  transaction.session().execute("delete from objects_viewport where objects_id = ? and user_id = ?").bind(ngcObject.id()).bind(user.id());
  transaction.session().add(new ViewPort{coordinates, angularSize, imageVersion, ngcObject, user});
}

DSSImage::ImageVersion ViewPort::imageVersion() const
{
  return _imageVersion;
}
