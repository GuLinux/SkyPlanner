#ifndef ASTRO_GROUP_H
#define ASTRO_GROUP_H
#include "Models"

#include "types.h"

struct AstroGroup {
  AstroSessionPtr _astroSession;
  NgcObjectPtr _object;
  AstroSessionObjectPtr astroSessionObject;
  TelescopePtr telescope;
  Timezone timezone;

  AstroGroup(const AstroSessionPtr &astroSession, const NgcObjectPtr &object, const TelescopePtr &telescope = {}, const Timezone &timezone = {})
    : _astroSession(astroSession), _object(object), telescope(telescope), timezone(timezone) {}
  AstroGroup(const AstroSessionObjectPtr &astroSessionObject, const TelescopePtr &telescope = {}, const Timezone &timezone = {})
    : astroSessionObject(astroSessionObject),telescope(telescope), timezone(timezone) {}

  AstroSessionPtr astroSession() const;
  NgcObjectPtr object() const;
};


#endif // ASTRO_GROUP_H
