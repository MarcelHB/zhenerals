#include "fmt/core.h"

#include "BattlefieldFactory.h"
#include "formats/MAPFile.h"
#include "InflatingStream.h"

namespace ZH {

BattlefieldFactory::BattlefieldFactory(ResourceLoader& mapLoader) : mapLoader(mapLoader) {}

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

  auto map = std::make_shared<Map>(std::move(*mapBuilder));

  return std::make_shared<Battlefield>(map);
}

}
