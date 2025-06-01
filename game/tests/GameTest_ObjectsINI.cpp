#include <algorithm>
#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../inis/ObjectsINI.h"

namespace ZH {

// Steam version v1.05, EN

void expectGenericTree(const Objects::Builder& object) {
  ASSERT_EQ(Objects::DrawType::MODEL_DRAW, object.drawMetaData.type);
  ASSERT_TRUE(object.drawMetaData.drawData);

  EXPECT_EQ(1, object.crushableLevel);

  EXPECT_EQ(1, object.armorSets.size());
  EXPECT_EQ("TreeArmor", object.armorSets.begin()->armor);

  EXPECT_TRUE(object.attributes.contains(Objects::Attribute::SHRUBBERY));
  EXPECT_TRUE(object.attributes.contains(Objects::Attribute::IMMOBILE));
  EXPECT_TRUE(object.attributes.contains(Objects::Attribute::IGNORED_IN_GUI));

  ASSERT_TRUE(object.body);
  ASSERT_EQ(object.body->type, Objects::ModuleType::HIGHLANDER_BODY);
  auto body = static_pointer_cast<Objects::ActiveBody>(object.body->moduleData);
  EXPECT_EQ(50.0f, body->maxHealth);
  EXPECT_EQ(50.0f, body->initialHealth);

  EXPECT_EQ(5, object.behaviors.size());
  auto result = std::find_if(object.behaviors.cbegin(), object.behaviors.cend(), [](const Objects::Behavior& b) {
    return b.type == Objects::ModuleType::SLOW_DEATH;
  });
  ASSERT_NE(result, object.behaviors.cend());
  auto slowDeathData = static_pointer_cast<Objects::SlowDeath>(result->moduleData);
  EXPECT_EQ(1, slowDeathData->deathTypes.size());
  EXPECT_TRUE(slowDeathData->deathTypes.contains(Objects::DeathType::TOPPLED));

  ASSERT_TRUE(object.clientUpdate);
  ASSERT_EQ(object.clientUpdate->type, Objects::ModuleType::SWAY_CLIENT);

  EXPECT_EQ(Objects::Geometry::CYLINDER, object.geometry.type);
  EXPECT_TRUE(object.geometry.small);
  EXPECT_EQ(15.0f, object.geometry.height);

  EXPECT_EQ(Objects::Shadow::DECAL, object.shadow.type);
  EXPECT_EQ(0.1f, object.scaleFuzziness);
}

TEST(ObjectsINITest, parsingNatureprop) {
  Config config;
  ResourceLoader w3dLoader {{"INIZH.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("data\\ini\\object\\natureprop.ini");
  auto stream = fileStream->getStream();
  ObjectsINI objectsINI {stream};

  auto objects = objectsINI.parse();
  EXPECT_EQ(179, objects.size());

  auto genericTreeLookup = objects.find("GenericTree");
  ASSERT_NE(genericTreeLookup, objects.cend());
  auto& genericTree = genericTreeLookup->second;

  expectGenericTree(genericTree);
  auto drawData = static_pointer_cast<Objects::ModelDrawData>(genericTree.drawMetaData.drawData);
  EXPECT_EQ("PTDogwod01", drawData->defaultConditionState.model);
  EXPECT_EQ(4, drawData->conditionStates.size());

  EXPECT_EQ(4.0f, genericTree.geometry.majorRadius);
  // ---------------
  auto palmLookup = objects.find("TreePalm1");
  ASSERT_NE(palmLookup, objects.cend());
  auto& palm = palmLookup->second;

  expectGenericTree(palm);
  drawData = static_pointer_cast<Objects::ModelDrawData>(palm.drawMetaData.drawData);
  EXPECT_EQ("PTPalm01", drawData->defaultConditionState.model);
  EXPECT_EQ(4, drawData->conditionStates.size());

  EXPECT_EQ(3.0f, palm.geometry.majorRadius);
}

}
