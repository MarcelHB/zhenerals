#ifndef H_GAME_OBJECT_LOADER
#define H_GAME_OBJECT_LOADER

#include "common.h"
#include "inis/ObjectsINI.h"
#include "ResourceLoader.h"

namespace ZH {

class ObjectLoader {
  public:
    ObjectLoader(ResourceLoader& iniLoader);

    bool init();
    std::shared_ptr<Objects::ObjectBuilder> getObject(const std::string&) const;
  private:
    ResourceLoader& iniLoader;
    ObjectsINI::ObjectMap index;
};

}

#endif
