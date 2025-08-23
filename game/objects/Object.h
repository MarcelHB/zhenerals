#ifndef H_GAME_OBJECT
#define H_GAME_OBJECT

#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <set>

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

// EVAL map?
struct UnitSpecificSounds {
  std::string afterburner;
  std::string deploy;
  std::string disguiseStarted;
  std::string disguiseRevealFailure;
  std::string disguiseRevealSuccess;
  std::string howitzerFire;
  std::string soundEject;
  std::string startDive;
  std::string stickyBombCreated;
  std::string truckLandingSound;
  std::string truckPowerslideSound;
  std::string undeploy;
  std::string underConstruction;

  std::string unitBombingPing;
  std::string unitCashPing;
  std::string unitPack;
  std::string unitUnpack;

  std::string voiceBombard;
  std::string voiceBuildResponse;
  std::string voiceCaptureBuildingComplete;
  std::string voiceClearBuilding;
  std::string voiceCombatDrop;
  std::string voiceCreate;
  std::string voiceCreated;
  std::string voiceCrush;
  std::string voiceDisableBuildingComplete;
  std::string voiceDisableVehicleComplete;
  std::string voiceDisarm;
  std::string voiceEject;
  std::string voiceEnter;
  std::string voiceEnterHostile;
  std::string voiceFireRocketPods;
  std::string voiceGarrison;
  std::string voiceGetHealed;
  std::string voiceHackInternet;
  std::string voiceLowFuel;
  std::string voiceMelee;
  std::string voiceMoveLoop;
  std::string voiceMoveUnderground;
  std::string voiceNoBuild;
  std::string voicePickup;
  std::string voicePoisonLocation;
  std::string voicePrimaryWeaponMode;
  std::string voiceRepair;
  std::string voiceRapidFire;
  std::string voiceSalvage;
  std::string voiceSecondaryWeaponMode;
  std::string voiceSnipePilot;
  std::string voiceStealCashComplete;
  std::string voiceSubdue;
  std::string voiceTertiaryWeaponMode;
  std::string voiceUnload;
  std::string voiceSupply;
};

struct WeaponPreference {
  std::string name; // TODO Weapon
  WeaponSlot slot;
  std::set<CommandSource> sources;
  std::set<Attribute> useAgainst;
};

struct WeaponSet {
  enum class Condition {
      NONE
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
  };
  std::set<Condition> conditions;
  std::list<WeaponPreference> weapons;
  bool sharedReloadTime = false;
  bool sharedLock = false;
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
  uint16_t buildTimeSec = 0;
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
  float extraWidth = 0.0f;
  bool forbidden = false;
  GeometryData geometry;
  std::shared_ptr<Behavior> inheritableModule;
  bool isBridge = false;
  std::set<Locomotor> locomotors;
  uint32_t occlusionDelay = 0;
  std::list<Behavior> overridableDefaults;
  float placementAngle = 0.0f;
  std::string portrait;
  bool prerequisiteForSomething = false;
  RadarPriority radarPriority = RadarPriority::NONE;
  Money refundValue = 0;
  float reservedExitWidth = 0.0f;
  uint8_t rubbleHeight = 1;
  float scale = 1.0f;
  float scaleFuzziness = 0.0f;
  std::list<std::string> sciencePrerequisites; // science names
  ShadowData shadow;
  bool simultaneousLimitByRestriction = false;
  std::optional<uint16_t> simultaneousLimit;
  std::list<std::string> objectPrerequisites; // object names
  std::string side;
  uint16_t threatValue = 1;
  bool trainable = false;

  bool transportable = false;
  uint8_t transportSlotCount = 0;
  std::list<std::string> upgradeCameo; // 1..5
  float visualRange = 10.0f;

  // EVAL map? (also merge with unit sounds?)
  std::string voiceAttack;
  std::string voiceAttackAir;
  std::string voiceAttackSpecial;
  std::string voiceCreated;
  std::string voiceDefect;
  std::string voiceEnter;
  std::string voiceEnemyEncounter;
  std::string voiceFear;
  std::string voiceGarrison;
  std::string voiceGroupSelect;
  std::string voiceMove;
  std::string voiceSelect;
  std::string voiceSelectElite;
  std::string voiceTaskDone;
  std::string voiceTaskInept;

  std::string soundAmbient;
  std::string soundAmbientDamaged;
  std::string soundAmbientSeverelyDamaged;
  std::string soundAmbientBroken;
  std::string soundCreated;
  std::string soundDamaged;
  std::string soundSeverelyDamaged;
  std::string soundEnter;
  std::string soundExit;
  std::string soundFallFromPlane;
  std::string soundMoveLoop;
  std::string soundMoveLoopDamaged;
  std::string soundMoveStart;
  std::string soundMoveStartDamaged;
  std::string soundPromotionVeteran;
  std::string soundPromotionElite;
  std::string soundPromotionHero;
  std::string soundStealthOn;
  std::string soundStealthOff;

  std::shared_ptr<UnitSpecificSounds> unitSounds;
  std::string unitCombatDropKillEffect; // UnitSpecificFX "list"
};

std::optional<Attribute> getAttribute(const std::string_view&);
std::optional<ArmorSet::Condition> getArmorSetCondition(const std::string_view&);
std::optional<Geometry> getGeometry(const std::string_view& value);
std::optional<ModelCondition> getModelCondition(const std::string_view&);
std::optional<DeathType> getDeathType(const std::string_view&);
std::optional<ModuleType> getModuleType(const std::string_view&);
std::optional<Shadow> getShadow(const std::string_view&);
std::optional<Status> getStatus(const std::string_view&);


}

#endif
