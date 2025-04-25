#include "Battlefield.h"

namespace ZH {

Battlefield::Battlefield(std::shared_ptr<Map>&& map)
  : map(std::move(map))
{}

std::shared_ptr<Map> Battlefield::getMap() const {
  return map;
}

}
