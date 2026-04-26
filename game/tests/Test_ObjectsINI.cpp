#include <set>

#include <gtest/gtest.h>

#include "../MurmurHash.h"
#include "../inis/ObjectsINI.h"

namespace ZH {

TEST(WaterINI, parsingDrawConditionStates) {
  std::ifstream stream {"tests/resources/ObjectsINI/objects.ini", std::ios::binary};

  ObjectsINI objectsINI {stream};
  auto map = objectsINI.parse();

  ASSERT_EQ(1, map.size());

  MurmurHash3_32 hasher;
  hasher.feed("FortMourne");
  auto lookup = map.find(hasher.getHash());
  ASSERT_TRUE(lookup != map.cend());

  auto& o1 = lookup->second;
  ASSERT_EQ(1, o1->drawMetaData.size());

  auto& d1 = o1->drawMetaData.front();
  EXPECT_EQ("ModuleTag01", d1.moduleTag);
  EXPECT_EQ(Objects::DrawType::MODEL_DRAW, d1.type);

  auto modelSpec = static_pointer_cast<const Objects::ModelDrawData>(d1.drawData);
  ASSERT_EQ(1, modelSpec->conditionStates.size());

  auto& cs1 = modelSpec->conditionStates.front();
  EXPECT_EQ(
      std::set<Objects::ModelCondition> {Objects::ModelCondition::NIGHT}
    , cs1.conditions
  );
}

}
