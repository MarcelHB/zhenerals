#ifndef H_GAME_BATTLEFIELD_FACTORY
#define H_GAME_BATTLEFIELD_FACTORY

#include "Battlefield.h"
#include "ResourceLoader.h"

namespace ZH {

class BattlefieldFactory {
  public:
    BattlefieldFactory(ResourceLoader& mapLoader);

    std::shared_ptr<Battlefield> load(const std::string&);
  private:
    ResourceLoader& mapLoader;
};

}

#endif
