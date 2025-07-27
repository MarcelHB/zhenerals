#ifndef H_GAME_OBJECTS_INI
#define H_GAME_OBJECTS_INI

#include <unordered_map>

#include "../common.h"
#include "INIFile.h"
#include "../objects/Object.h"

namespace ZH {

class ObjectsINI : public INIFile {
  public:
    using ObjectMap =
      std::unordered_map<std::string, std::shared_ptr<Objects::ObjectBuilder>>;

    ObjectsINI(std::istream&);
    ObjectMap parse();

  private:
    bool parseObject(ObjectMap&);

  public:
    bool parseBehavior(Objects::ObjectBuilder&);
    bool parseBody(Objects::ObjectBuilder&);
    bool parseClientUpdate(Objects::ObjectBuilder&);
    bool parseDraw(Objects::ObjectBuilder&);
};

}

#endif
