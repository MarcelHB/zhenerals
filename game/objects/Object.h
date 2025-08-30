#ifndef H_GAME_OBJECT
#define H_GAME_OBJECT

#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

#include "../common.h"
#include "../Color.h"
#include "Drawing.h"
#include "Modules.h"

namespace ZH::Objects {

struct ArmorSet {
  enum class Condition {
      NONE = 0
    , VETERAN
    , ELITE
    , HERO
    , PLAYER_UPGRADE
    , WEAK_VS_BASE_DEFENSE
    , SECOND_LIFE
    , CRATE_UPGRADE_ONE
    , CRATE_UPGRADE_TWO
    , ALL
  };
  std::set<Condition> conditions;
  std::string armor; // TODO Armor
  std::string damage; // TODO DamageFX
};

struct Behavior {
  std::string moduleTag;
  ModuleType type;
  std::shared_ptr<Module> moduleData;
};

struct DrawMetaData {
  std::string moduleTag;
  DrawType type;
  std::shared_ptr<DrawData> drawData;
};

struct GeometryData {
  Geometry type = Geometry::NONE;
  bool small = false;
  float height = 1.0f;
  float minorRadius = 1.0f;
  float majorRadius = 1.0f;
};

struct ShadowData {
  Shadow type = Shadow::NONE;
  glm::vec2 size;
  glm::vec2 offset;
  std::string texture;
};

struct WeaponPreference {
  std::string name; // TODO Weapon
  WeaponSlot slot;
  std::set<CommandSource> sources;
  std::set<Attribute> useAgainst;
};

struct WeaponSet {
  enum class Condition {
      NONE = 0
    , VETERAN
    , ELITE
    , HERO
    , PLAYER_UPGRADE
    , CRATE_UPGRADE_ONE
    , CRATE_UPGRADE_TWO
    , HIJACK
    , CAR_BOMB
    , MONE_CLEARNING
    , RIDER1
    , RIDER2
    , RIDER3
    , RIDER4
    , RIDER5
    , RIDER6
    , RIDER7
    , RIDER8
    , ALL
  };
  std::set<Condition> conditions;
  std::array<WeaponPreference, 3> weapons;
  bool sharedReloadTime = false;
  bool sharedLock = false;
};

struct Locomotor {
  LocomotorType type;
  std::string locomotor; // TODO Locomotor
};

struct ObjectBuilder {
  std::string name;
  std::list<Behavior> behaviors;
  std::shared_ptr<Behavior> body;
  std::shared_ptr<Behavior> clientUpdate;
  DrawMetaData drawMetaData;

  std::list<ArmorSet> armorSets;
  std::list<WeaponSet> weaponSets;

  bool buildable = false;
  Money buildCost = 0;
  uint16_t buildTimeSec = 1; // EVAL float?
  std::list<std::string> buildVariations; // object names
  std::set<Attribute> attributes;
  std::string buttonImage;
  bool canEnterWhenGuarding = false;
  bool canHijackWhenGuarding = false;
  float cloakRange = 0.0f; // EVAL shroudRange
  Color color;
  std::string commandSet; // TODO CommandSet
  CompletionAppearance completionAppearance = CompletionAppearance::NONE;
  uint8_t crushableLevel = 1;
  uint8_t crushingLevel = 1;
  float decloakedRange = 0.0f; // EVAL shroudRevealToAllRange
  float decloakingRange = 0.0f; // EVAL shroudClearingRange
  std::u16string displayName;
  Energy energyContribution = 0;
  Energy energyBonus = 0;
  std::array<uint16_t, 4> experienceValues;
  std::array<uint16_t, 4> experienceRequirements;
  float factoryExitWidth = 0.0f;
  bool forbidden = false;
  GeometryData geometry;
  std::shared_ptr<Behavior> inheritableModule;
  bool isBridge = false;
  std::vector<Locomotor> locomotors;
  uint32_t occlusionDelay = 0;
  std::list<Behavior> overridableDefaults;
  float placementAngle = 0.0f; // deg
  std::string portrait;
  bool prerequisiteForSomething = false;
  RadarPriority radarPriority = RadarPriority::NONE;
  Money refundValue = 0;
  float reservedExitWidth = 0.0f;
  uint8_t rubbleHeight = 1;
  float scale = 1.0f;
  float scaleFuzziness = 0.0f;
  std::string sciencePrerequisite; // science names
  ShadowData shadow;
  std::string simultaneousLimitRestrictionByKey;
  std::optional<uint16_t> simultaneousLimit;
  bool simultaneousLimitByRestriction = false;
  std::list<std::string> objectPrerequisites; // object names
  std::string side;
  uint16_t threatValue = 1;
  bool trainable = false;

  bool transportable = false;
  std::optional<uint8_t> transportSlotCount = {};
  std::array<std::string, 5> upgradeCameos; // TODO Upgrades
  float visualRange = 10.0f;

  std::unordered_map<Noise, std::string> noises;

  std::string unitCombatDropKillEffect; // UnitSpecificFX "list"
};

std::optional<AnimationMode> getAnimationMode(const std::string_view&);
std::optional<AnimationFrameMode> getAnimationFrameMode(const std::string_view&);
std::optional<Attribute> getAttribute(const std::string_view&);
std::optional<ArmorSet::Condition> getArmorSetCondition(const std::string_view&);
std::optional<AutoAcquireEnemyMode> getAutoAcquireEnemyMode(const std::string_view&);
std::optional<CommandSource> getCommandSource(const std::string_view&);
std::optional<CompletionAppearance> getCompletionAppearance(const std::string_view&);
std::optional<DamageType> getDamageType(const std::string_view&);
std::optional<DeathType> getDeathType(const std::string_view&);
std::optional<Geometry> getGeometry(const std::string_view& value);
std::optional<LocomotorType> getLocomotorType(const std::string_view& value);
std::optional<MaxHealthModifier> getMaxHealthModifier(const std::string_view& value);
std::optional<ModelCondition> getModelCondition(const std::string_view&);
std::optional<ModuleType> getModuleType(const std::string_view&);
std::optional<OCLLocation> getOCLLocation(const std::string_view&);
std::optional<RadarPriority> getRadarPriority(const std::string_view&);
std::optional<Shadow> getShadow(const std::string_view&);
std::optional<SlowDeathPhase> getSlowDeathPhase(const std::string_view&);
std::optional<Status> getStatus(const std::string_view&);
std::optional<StealthLevel> getStealthLevel(const std::string_view&);
std::optional<StructureCollapsePhase> getStructureCollapsePhase(const std::string_view&);
std::optional<Veterancy> getVeterancy(const std::string_view&);
std::optional<WeaponSlot> getWeaponSlot(const std::string_view&);
std::optional<WeaponSet::Condition> getWeaponSetCondition(const std::string_view&);


}

#endif
