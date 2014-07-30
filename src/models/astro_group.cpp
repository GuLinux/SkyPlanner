#include "Models"

AstroSessionPtr AstroGroup::astroSession() const
{
  if(astroSessionObject)
    return astroSessionObject->astroSession();
  return _astroSession;
}

NgcObjectPtr AstroGroup::object() const
{
  if(astroSessionObject)
    return astroSessionObject->ngcObject();
  return _object;
}
