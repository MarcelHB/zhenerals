#ifndef H_GAME_BATTLEFIELD_FACTORY
#define H_GAME_BATTLEFIELD_FACTORY

#include "Battlefield.h"
#include "objects/InstanceFactory.h"
#include "ResourceLoader.h"

namespace ZH {

class BattlefieldFactory {
  public:
    BattlefieldFactory(
        ResourceLoader& mapLoader
      , Objects::InstanceFactory& instanceFactory
    );

    std::shared_ptr<Battlefield> load(const std::string&);
  private:
    ResourceLoader& mapLoader;
    Objects::InstanceFactory& instanceFactory;
};

}

#endif
