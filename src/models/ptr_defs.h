#ifndef PTR_DEFS_H
#define PTR_DEFS_H


#include <Wt/Dbo/ptr>

class AstroSession;
class AstroSessionObject;
class Telescope;
class NgcObject;
class NebulaDenomination;
class ViewPort;
class User;
class Catalogue;
class UserSetting;

typedef Wt::Dbo::ptr<AstroSession> AstroSessionPtr;
typedef Wt::Dbo::ptr<AstroSessionObject> AstroSessionObjectPtr;
typedef Wt::Dbo::ptr<Telescope> TelescopePtr;
typedef Wt::Dbo::ptr<NgcObject> NgcObjectPtr;
typedef Wt::Dbo::ptr<NebulaDenomination> NebulaDenominationPtr;
typedef Wt::Dbo::ptr<ViewPort> ViewPortPtr;
typedef Wt::Dbo::ptr<User> UserPtr;
typedef Wt::Dbo::ptr<Catalogue> CataloguePtr;
typedef Wt::Dbo::ptr<UserSetting> UserSettingPtr;

#endif // PTR_DEFS_H
