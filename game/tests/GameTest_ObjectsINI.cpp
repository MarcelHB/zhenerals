#include <algorithm>
#include <gtest/gtest.h>

#include "../Config.h"
#include "../MurmurHash.h"
#include "../ResourceLoader.h"
#include "../inis/ObjectsINI.h"

namespace ZH {

// Steam version v1.05, EN

void expectGenericTree(const Objects::ObjectBuilder& builder) {
  ASSERT_EQ(Objects::DrawType::MODEL_DRAW, builder.drawMetaData.front().type);
  ASSERT_TRUE(builder.drawMetaData.front().drawData);

  EXPECT_EQ(1, builder.crushableLevel);

  EXPECT_EQ(1, builder.armorSets.size());
  EXPECT_EQ("TreeArmor", builder.armorSets.begin()->armor);

  EXPECT_TRUE(builder.attributes.contains(Objects::Attribute::SHRUBBERY));
  EXPECT_TRUE(builder.attributes.contains(Objects::Attribute::IMMOBILE));
  EXPECT_TRUE(builder.attributes.contains(Objects::Attribute::IGNORED_IN_GUI));

  ASSERT_TRUE(builder.body);
  ASSERT_EQ(builder.body->type, Objects::ModuleType::HIGHLANDER_BODY);
  auto body = static_pointer_cast<Objects::ActiveBody>(builder.body->moduleData);
  EXPECT_EQ(50.0f, body->maxHealth);
  EXPECT_EQ(50.0f, body->initialHealth);

  EXPECT_EQ(5, builder.behaviors.size());
  auto result = std::find_if(builder.behaviors.cbegin(), builder.behaviors.cend(), [](const Objects::Behavior& b) {
    return b.type == Objects::ModuleType::SLOW_DEATH;
  });
  ASSERT_NE(result, builder.behaviors.cend());
  auto slowDeathData = static_pointer_cast<Objects::SlowDeath>(result->moduleData);
  EXPECT_EQ(1, slowDeathData->deathTypes.size());
  EXPECT_TRUE(slowDeathData->deathTypes.contains(Objects::DeathType::TOPPLED));

  ASSERT_TRUE(builder.clientUpdate);
  ASSERT_EQ(builder.clientUpdate->type, Objects::ModuleType::SWAY_CLIENT);

  EXPECT_EQ(Objects::Geometry::CYLINDER, builder.geometry.type);
  EXPECT_TRUE(builder.geometry.small);
  EXPECT_EQ(15.0f, builder.geometry.height);

  EXPECT_EQ(Objects::Shadow::DECAL, builder.shadow.type);
  EXPECT_EQ(0.1f, builder.scaleFuzziness);
}

TEST(ObjectsINITest, parsingNatureProp) {
  Config config;
  ResourceLoader w3dLoader {{"INIZH.big"}, config.baseDir};

  auto fileStream =
    w3dLoader.getFileStream("data\\ini\\object\\natureprop.ini");
  auto stream = fileStream->getStream();
  ObjectsINI objectsINI {stream};

  auto builders = objectsINI.parse();
  ASSERT_FALSE(objectsINI.hasErroneousObject());
  EXPECT_EQ(179, builders.size());

  MurmurHash3_32 hasher1;
  hasher1.feed("GenericTree");

  auto genericTreeLookup = builders.find(hasher1.getHash());
  ASSERT_NE(genericTreeLookup, builders.cend());
  auto& genericTree = *genericTreeLookup->second;

  expectGenericTree(genericTree);
  auto drawData = static_pointer_cast<Objects::ModelDrawData>(genericTree.drawMetaData.front().drawData);
  EXPECT_EQ("PTDogwod01", drawData->defaultConditionState.model);
  EXPECT_EQ(4, drawData->conditionStates.size());

  EXPECT_EQ(4.0f, genericTree.geometry.majorRadius);
  // ---------------

  MurmurHash3_32 hasher2;
  hasher2.feed("TreePalm1");

  auto palmLookup = builders.find(hasher2.getHash());
  ASSERT_NE(palmLookup, builders.cend());
  auto& palm = *palmLookup->second;

  expectGenericTree(palm);
  drawData = static_pointer_cast<Objects::ModelDrawData>(palm.drawMetaData.front().drawData);
  EXPECT_EQ("PTPalm01", drawData->defaultConditionState.model);
  EXPECT_EQ(4, drawData->conditionStates.size());

  EXPECT_EQ(3.0f, palm.geometry.majorRadius);
}

class ParamObjectsINITest : public testing::TestWithParam<std::string> {
protected:
  ResourceLoader::MemoryStream stream;
  MemoryViewStream streamView {nullptr, 0};
  ObjectsINI *unit;

  static ResourceLoader *w3dLoader;

public:
  void SetUp() override {
    stream = *w3dLoader->getFileStream(GetParam());
    streamView = std::move(stream.getStream());
    unit = new ObjectsINI {streamView};
  }

  void TearDown() override {
    delete unit;
  }

  static void SetUpTestSuite() {
    Config config;
    w3dLoader = new ResourceLoader {{"INIZH.big"}, config.baseDir};
  }

  static void TearDownTestSuite() {
    delete w3dLoader;
  }
};

ResourceLoader* ParamObjectsINITest::w3dLoader = nullptr;

TEST_P(ParamObjectsINITest, parse) {
  unit->parse();
  ASSERT_FALSE(unit->hasErroneousObject());
}

INSTANTIATE_TEST_SUITE_P(
    ObjectsINIInstances
  , ParamObjectsINITest
  , testing::Values(
        "data\\ini\\object\\airforcegeneral.ini"
      , "data\\ini\\object\\americavehicle.ini"
      , "data\\ini\\object\\chinavehicle.ini"
      , "data\\ini\\object\\civilianbuilding.ini"
      , "data\\ini\\object\\civilianprop.ini"
      , "data\\ini\\object\\civilianunit.ini"
      , "data\\ini\\object\\factionbuilding.ini"
      , "data\\ini\\object\\natureprop.ini"
      , "data\\ini\\object\\techbuildings.ini"
    )
  , [](const testing::TestParamInfo<ParamObjectsINITest::ParamType>& info) {
      std::string name {info.param};
      auto pos = name.find("\\");
      while (std::string::npos != pos) {
        name[pos] = '_';
        pos = name.find("\\");
      }

      pos = name.find(".");
      name[pos] = '_';

      return name;
    }
);

}
