// SPDX-License-Identifier: GPL-2.0

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
      std::unordered_map<uint32_t, std::shared_ptr<Objects::ObjectBuilder>>;

    ObjectsINI(std::istream&);
    ObjectMap parse();

  private:
    bool erroneousObject = false;
    bool parseObject(ObjectMap&, bool reskin = false);

  public:
    bool hasErroneousObject() const;
    bool parseBehavior(Objects::ObjectBuilder&);
    bool parseBody(Objects::ObjectBuilder&);
    bool parseClientUpdate(Objects::ObjectBuilder&);
    std::set<Objects::ModelCondition> parseConditionStateConditions();
    bool parseDraw(Objects::ObjectBuilder&);
};

}

#endif
