#include "BattlefieldFactory.h"
#include "formats/MAPFile.h"
#include "InflatingStream.h"

namespace ZH {

BattlefieldFactory::BattlefieldFactory(
    ResourceLoader& mapLoader
) : mapLoader(mapLoader)
{}

std::shared_ptr<Battlefield> BattlefieldFactory::load(const std::string& mapFileName) {
  std::string path {"maps\\"};
  path.append(mapFileName);
  path.append("\\");
  path.append(mapFileName);
  path.append(".map");

  auto lookup = mapLoader.getFileStream(path);
  if (!lookup) {
    return {};
  }

  auto stream = lookup->getStream();
  InflatingStream inflatingStream {stream};

  MAPFile mapFile {inflatingStream};
  auto map = mapFile.parseMap();
  if (!map) {
    return {};
  }

  return std::make_shared<Battlefield>(std::move(map));
}

}
