#ifndef H_GAME_OBJECTS_INI
#define H_GAME_OBJECTS_INI

#include <unordered_map>

#include "INIFile.h"
#include "../objects/Object.h"

namespace ZH {

class ObjectsINI : public INIFile {
  public:
    using ObjectMap = std::unordered_map<std::string, Objects::Builder>;

    ObjectsINI(std::istream&);
    ObjectMap parse();

  private:
    bool parseObject(ObjectMap&);

  public:
    bool parseBehavior(Objects::Builder&);
    bool parseBody(Objects::Builder&);
    bool parseClientUpdate(Objects::Builder&);
    bool parseDraw(Objects::Builder&);
};

}

#endif
