#include "fmt/core.h"

#include "BattlefieldFactory.h"
#include "formats/MAPFile.h"
#include "InflatingStream.h"

namespace ZH {

BattlefieldFactory::BattlefieldFactory(
    ResourceLoader& mapLoader
  , Objects::InstanceFactory& instaceFactory
) : mapLoader(mapLoader)
  , instanceFactory(instanceFactory)
{}

std::shared_ptr<Battlefield> BattlefieldFactory::load(const std::string& mapFileName) {
  std::string path = fmt::format("maps\\{}\\{}.map", mapFileName, mapFileName);

  auto lookup = mapLoader.getFileStream(path);
  if (!lookup) {
    return {};
  }

  auto stream = lookup->getStream();
  InflatingStream inflatingStream {stream};

  MAPFile mapFile {inflatingStream};
  auto mapBuilder = mapFile.parseMap();
  if (!mapBuilder) {
    return {};
  }

  auto map = std::make_shared<Map>(*mapBuilder);

  return std::make_shared<Battlefield>(
      map
    , *mapBuilder
    , instanceFactory
  );
}

}
