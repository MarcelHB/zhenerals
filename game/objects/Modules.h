// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_OBJECT_MODULE
#define H_GAME_OBJECT_MODULE

#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <set>
#include <string>

#include <glm/glm.hpp>

#include "../common.h"
#include "Attributes.h"
#include "../Color.h"

namespace ZH::Objects {

struct Module {
};

enum class ModuleType {
    ACTIVE_BODY
  , AI
  , ANIMATED_PARTICLE_SYS_BONE_CLIENT
  , ANIMATION_STEERING
  , ARMOR_UPGRADE
  , ASSAULT_TRANSPORT
  , ASSISTED_TARGETING
  , AUTO_DEPOSIT
  , AUTO_FIND_HEALING
  , AUTO_HEAL
  , BAIKONUR_LAUNCH_POWER
  , BASE_REGENERATE
  , BATTLE_BUS_SLOW_DEATH
  , BATTLE_PLAN
  , BONE_FX
  , BONE_FX_DAMAGE
  , BRIDGE
  , BRIDGE_TOWER
  , BUNKER_BUSTER
  , BODY
  , CASH_BOUNTY
  , CASH_HACK
  , CAVE_CONTAIN
  , CHECKPOINT
  , CHINOOK_AI
  , CLEANUP_AREA
  , CLEANUP_HAZARD
  , COMMAND_BUTTON_HUNT
  , COMMAND_SET_UPGRADE
  , CONVERT_TO_CAR_BOMB
  , CONVERT_TO_HIJACK
  , COST_MODIFIER_UPGRADE
  , COUNTERMEASURE
  , CRATE_COLLISION
  , CREATE
  , CREATE_CRATE_DIE
  , CREATE_OBJECT_DIE
  , CRUSH_DIE
  , DAMAGE
  , DAM_DIE
  , DEFAULT_PRORDUCTION_EXIT
  , DEFECTOR_SPECIAL
  , DELETION
  , DELIVER_PAYLOAD
  , DEMO_TRAP
  , DEPLOY_STYLE_AI
  , DESTROY_DIE
  , DIE
  , DOCK
  , DOZER_AI
  , DUMB_PROJECTILE
  , DYNAMIC_GEOMETRY_INFO
  , DYNAMIC_SHROUD_CLEARING_RANGE
  , EJECT_PILOT_DIE
  , EMP
  , ENEMY_NEAR
  , EXPERINCE_SCALAR_UPGRADE
  , FIRE_OCL_AFTER_WEAPON_COOLDOWN
  , FIRE_SPREAD
  , FIRESTORM_DYNAMIC_GEOMETRY_INFO
  , FIRE_WEAPON_COLLISION
  , FIRE_WEAPON_POWER
  , FIRE_WEAPON
  , FIRE_WEAPON_WHEN_DAMAGED
  , FIRE_WEAPON_WHEN_DEAD
  , FLAMMABLE
  , FLIGHT_DECK
  , FLOAT
  , FX_LIST_DIE
  , GARRISON_CONTAIN
  , GENERATE_MINEFIELD
  , GRANT_SCIENCE_UPGRADE
  , GRANT_STEALTH
  , GRANT_UPGRADE
  , HACK_INTERNET
  , HEAL_CONTAIN
  , HEIGHT_DIE
  , HELICOPTER_SLOW_DEATH
  , HELIX_CONTAIN
  , HIGHLANDER_BODY
  , HIVE_STRUCTURE_BODY
  , IMMORTAL_BODY
  , INSTANT_DEATH
  , JET_AI
  , JET_SLOW_DEATH
  , KEEP_OBJECT_DIE
  , LASER
  , LEAFLET_DROP
  , LIFETIME
  , LOCK_WEAPON
  , LOCOMOTOR_SET_UPGRADE
  , MAX_HEALTH_UPGRADE
  , MISSILE_AI
  , MONEY_CRATE_COLLISION
  , NEUTRON_MISSILE_SLOW_DEATH
  , OBJECT_CREATION_UPGRADE
  , OCL
  , OCL_SPECIAL_POWER
  , OPEN_CONTAIN
  , OVERLORD_CONTAIN
  , PARACHUTE_CONTAIN
  , PARKING_PLACE
  , PARTICLE_UPLINK_CANNON
  , PREORDER_CREATE
  , PHYSICS
  , PILOT_FIND_VEHICLE
  , POINT_DEFENSE_LASER
  , POISONED
  , POWER_PLANT
  , POWER_PLANT_UPGRADE
  , PRODUCTION
  , RADAR
  , RADAR_UPGRADE
  , REPAIR_DOCK
  , SALVAGE_CRATE_COLLISION
  , SLAVED
  , SLOW_DEATH
  , SPAWN
  , SPECIAL_POWER
  , SPECIAL_POWER_COMPLETION_DIE
  , SPECIAL_POWER_CREATE
  , SPECIAL_POWER_UPDATE
  , SPECTRE_GUNSHIP
  , SPECTRE_GUNSHIP_DEPLOYMENT
  , SPY_VISION
  , SPY_VISION_SPECIAL_POWER
  , SQUISH_COLLIDE
  , SUPPLY_CENTER
  , SUPPLY_CENTER_DOCK
  , SUPPLY_CENTER_PRODUCTION_EXIT
  , SUPPLY_TRUCK_AI
  , SUPPLY_WAREHOUSE
  , SUPPLY_WAREHOUSE_CRIPPLING
  , SUPPLY_WAREHOUSE_DOCK
  , STEALTH
  , STEALTH_UPGRADE
  , STEALTH_DETECTOR
  , STRUCTURE_BODY
  , STRUCTURE_COLLAPSE
  , STRUCTURE_TOPPLE
  , SWAY_CLIENT
  , TECH_BUILDING
  , TOPPLE
  , TRANSITION_DAMAGE_FX
  , TRANSPORT_AI
  , TRANSPORT_CONTAIN
  , UNPAUSE_SPECIAL_POWER_UPGRADE
  , UPGRADE
  , UPGRADE_DIE
  , VETERANCY_GAIN
  , VETERANCY_CRATE_COLLISION
  , WAVE_GUIDE
  , WEAPON_BONUS
  , WEAPON_SET_UPGRADE
};

struct Turret {
  bool canPitch = true;
  bool canFireAndTurn = true;
  float defaultAngle = 0.0f;
  float defaultPitch = 0.0f;
  bool disabled = false;
  std::set<WeaponSlot> controlledSlots;
  float firePitch = 0.0f;
  std::array<std::optional<float>, 3> fireAngleSweep;
  float groundUnitPitch = 0.0f;
  float maxScanAngle = 0.0f;
  Duration maxScanIntervalMs = 1;
  float minPitch = 0.0f;
  float minScanAngle = 0.0f;
  Duration minScanIntervalMs = 1;
  Duration recenterTimeMs = 1;
  glm::vec2 rotationRate; // deg/sec
  std::array<float, 3> sweepSpeedModifier;
};

struct ActiveBody : public Module {
  Health maxHealth = 0.0f;
  Health initialHealth = 0.0f;
  float subdualDamageCap = 0.0f;
  Duration subdualDamageHealRateMs = 0;
  float subdualDamageHealAmount = 0.0f;
};

struct AI : public Module {
  std::set<AutoAcquireEnemyMode> acquireEnemiesWhenIdle;
  bool ignorePlayer = false;
  Duration moodAttackCheckRateMs = 250;
  Turret turret1;
  Turret turret2;
  bool turretsLinked = false;
};

struct AnimationSteering : public Module {
  uint32_t minTransitionTime = 0;
};

struct AssaultTransport : public AI {
  Percent healedWhenBelow = 50; // float!
  bool clearRangeForAttackMove = true;
};

struct AssistedTargeting : public Module {
  int32_t numShots = 1;
  WeaponSlot slot = WeaponSlot::NONE;
  std::string laserFrom; // object names
  std::string laserTo;
};

struct AutoDeposit : public Module {
  Duration intervalMs;
  Money amount;
  Money captureBonus;
  bool actualMoney;
  std::string upgrade; // TODO Upgrade
  int32_t boostValue = 1;
};

struct AutoFindHealing : public Module {
  float alwaysHeal = 1.0f;
  float neverHeal = 100.0f; // percent
  Duration scanIntervalMs = 1000;
  float scanRange = 10.0f;
};

struct AutoHeal : public Module {
  bool enabled = true;
  bool singleBurst = false;
  int32_t healingAmount = 1;
  Duration healingDelayMs = 0;
  float radius = 10.0f;
  std::set<Attribute> healingInclusion;
  std::set<Attribute> healingExclusion;
  std::string radiusParticleSystem; // TODO ParticleSystem
  std::string healParticleSyste;
  Duration startHealingDelayMs = 0;
  bool wholePlayer = true;
  bool skipSelf = false;
};

struct BattlePlan : public Module {
  // Hold The Line, Search aNd Destory, BomBardment
  std::string specialPower; // TODO SpecialPower
  uint32_t bbAnimationTime = 3;
  uint32_t htlAnimationTime = 3;
  uint32_t sndAnimationTime = 3;
  uint32_t transitionIdleTime = 0;
  std::string bbUnpackSound;
  std::string bbPackSound;
  std::string bbMessage;
  std::string bbAnnouncement;
  std::string htlUnpackSound;
  std::string htlPackSound;
  std::string htlMessage;
  std::string htlAnnouncement;
  std::string sndUnpackSound;
  std::string sndPackSound;
  std::string sndMessage;
  std::string sndAnnouncement;

  std::set<Attribute> memberInclusion;
  std::set<Attribute> memberExclusion;
  uint32_t planChangeTime = 3;
  float hdlFactor = 1.0f;
  float hdlBuildingHealthFactor = 1.0f;
  MaxHealthModifier hdlBuildingHealthModifier = MaxHealthModifier::PRESERVE_RATIO;
  float sndFactor = 1.0f;
  float sndBuildingFactor = 1.0f;
  bool sndBuildingStealthDetection = true;
  std::string vision; // object name
};

struct Body : public Module {
  float maxHealth = 1.0f;
  float initialHealth = 1.0f;
  float subdueDamageCap = 1.0f;
  uint32_t subdueDamageHealRate = 1;
  float subdueDamageHealAmount = 1.0f;
};

// EVAL naming

struct BoneFXItem {
  std::string bone;
  std::string itemName; // TODO particle system/OCL/FXList
  Duration maxDelayMs = 1000;
  Duration minDelayMs = 0;
  bool once;
};

using BoneFXItems = std::array<std::array<BoneFXItem, 4>, 8>;

struct BoneFX : public Module {
  BoneFXItems creationLists;
  std::set<DamageType> damageEffectTypes;
  std::set<DamageType> damageParticleTypes;
  BoneFXItems effects;
  BoneFXItems particles;
};

struct BridgeDieItem {
  std::string name; // TODO FXList/OCL
  Duration delayMs = 1000;
  std::string bone;
};

struct Bridge : public Module {
  std::list<BridgeDieItem> dieCreationLists;
  std::list<BridgeDieItem> dieEffects;
  float lateralScaffoldSpeed = 1.0f;
  float verticalScaffoldSpeed = 1.0f;
};

struct BunkerBuster : public Module {
  std::string requiredUpgrade; // TODO Upgrade
  std::string detonationEffect; // TODO FXList?
  std::string crashThroughEffect; // TODO FXList?
  uint32_t crashThroughFrequency = 1;
  float seismicRadius = 10.0f;
  float seismicMagnitude = 10.0f;
  std::string shockwaveWeapon; // TODO Weapon
  std::string occupantDamageWeapon;
};

struct Checkpoint {
  uint32_t scanDelay = 0;
};

struct CleanupHazard : public Module {
  WeaponSlot slot;
  uint32_t scanRate = 1;
  float scanRange = 10.0f;
};

struct CommandButtonHunt : public Module {
  uint32_t scanRate = 1;
  float scanRange = 10.0f;
};

struct CrateCollision : public Module {
  bool buildingCanPickUp = false; // EVAL
  std::set<Attribute> collisionInclusion;
  std::set<Attribute> collisionExclusion;
  std::string collisionAnimation; // TODO animation
  float collisionAnimationDurationSec = 1.0f;
  float collisionAnimationRisePerSec = 10.0f;
  bool collisionAnimationFadeOut = true;
  bool onlyPlayerCanPickUp = false;
  bool ownerCanPickUp = true;
  std::string sciencePrerequisite; // TODO Science
};

struct ConvertToCarBomb : public CrateCollision {
  std::string effect; // TODO FXList
};

struct MoneyCrateCollision : public CrateCollision {
  Money value = 1;
  std::string upgradeToBoost; // TODO Upgrade
  uint8_t boostValue = 1;
};

struct Countermeasure : public Module {
  std::string flare;
  std::string flareBone;
  uint32_t volleySize = 1;
  float volleyArcAngle = 0.0f;
  float volleyVelocityFactor = 1.0f;
  uint32_t volleyInterval = 1;
  uint32_t numVolleys = 1;
  uint32_t reloadTime = 1;
  Percent evasionChance = 50;
  bool reloadAground = false;
  uint32_t missleDecoyDelay = 1;
  uint32_t reactionLatency = 1;
};

struct Die : public Module {
  std::set<DeathType> deathTypes;
  std::set<Status> excludedStates;
  std::set<Status> requiredStates;
  std::set<Veterancy> veterancyLevels;
};

struct CreateCrateDie : public Die {
  std::string crate; // TODO Crate
};

struct CreateObjectDie : public Die {
  std::string creationList; // TODO ObjectCreationList
  bool transferHealth = false;
};

struct CrushDie : public Die {
  std::string crushSound;
  std::string backEndCrushSound;
  std::string frontEndCrushSound;
  Percent totalCrushSound;
  Percent totalBackEndSound;
  Percent totalFrontEndSound;
};

struct DestroyDie : public Die {
};

struct EjectPilotDie : public Die {
  std::string airCreationList; // TODO ObjectCreationList
  std::string groundCreationList;
  uint32_t invulnerableTime = 5;
};

struct FXListDie : public Die {
  bool active = true;
  std::string effect; // TODO FXList
  bool orientToObject = true;
};

struct SpecialPowerCompletionDie : public Die {
  std::string specialPower; // TODO SpecialPower
};

struct DefaultProductionExit : public Module {
  glm::vec3 creationPoint;
  glm::vec3 rallyPoint;
  bool useRallyPoint = false;
};

struct Deletion : public Module {
  Duration minLifetimeMs;
  Duration maxLifetimeMs;
};

struct DeliverPayload : public AI {
  int32_t attempts = 3;
  std::string decal; // TODO Decal
  float decalRadius = 2.0f;
  float distance = 1.0f;
  Duration doorDelayMs = 1000;
  std::string dropCarrier; // object name
  Duration dropDelayMs = 1;
  glm::vec3 offset;
  glm::vec3 variance;
};

struct DemoTrap : public Module {
  bool defaultProximity = true;
  WeaponSlot detonation;
  WeaponSlot proximity;
  WeaponSlot manual;
  float triggerRange = 3.0f;
  std::set<Attribute> triggerExclusions;
  uint32_t scanRate;
  bool detonateWithAllies = true;
  std::string detonationWeapon; // TODO Weapon
  bool detonateOnDeath = true;
};

struct DeployStyleAI : public AI {
  Duration unpackTimeMs = 1000;
  Duration packTimeMs = 1000;
  bool resetTurretsBeforePacking = true;
  bool deployRequired = true;
  bool centerBeforePacking = true;
  bool manualDeployAnimation = false;
};

struct Dock : public Module {
  int32_t numApproachPositions = 1;
  bool allowPassThrough = false;
};

struct DozerAI : public AI {
  float boredRange = 5.0f;
  float boredTime = 5.0f; // EVAL frames? ms?
  float repairPerSecond = 2.0f;
};

struct DumbProjectile : public Module {
  uint32_t maxLifespan = 1;
  float adjustDistPerSecond = 1.0f;
  bool randomTumble = false;
  bool dieOnDetonation = true;
  bool orientToPath = true;
  float firstHeight = 1.0f;
  float secondHeight = 1.0f;
  Percent firstIndent = 0;
  Percent secondIndent = 0;

  std::set<Attribute> garrisonKillInclusion;
  std::set<Attribute> garrisonKillExclusion;
  uint32_t garrisonKillCount = 10;
  std::string garrisonKillEffect; // TODO FXList
};

struct DynamicGeometryInfo : public Module {
  uint32_t initialDelay = 0;
  float initialHeight = 0.0f;
  float initialMinorRadius = 0.0f;
  float initialMajorRadius = 0.0f;
  float finalHeight = 0.0f;
  float finalMinorRadius = 0.0f;
  float finalMajorRadius = 0.0f;
  uint32_t transitionTime = 1;
  bool reverseAtTransitionTime = false;
};

struct RadiusDecal {
  std::string texture;
  Shadow shadow;
  Percent minOpacity = 0;
  Percent maxOpacity = 100;
  uint32_t opacityThrobTime = 1;
  Color color;
};

struct DynamicShroudClearingRange : public Module {
  uint32_t changeInterval = 1;
  uint32_t growInterval = 1;
  uint32_t shrinkDelay = 2;
  uint32_t shrinkTime = 1;
  uint32_t growDelay = 2;
  uint32_t growTime = 1;
  float finalVision = 1.0f;
  RadiusDecal radiusDecal;
};

struct EMP : public Module {
  uint32_t lifetime = 10;
  uint32_t startFadeAfter = 10;
  float startScale = 1.0f;
  uint32_t disabledDuration = 0;
  float minTargetScale = 1.0f;
  float maxTargetScale = 2.0f;
  Color startColor;
  Color endColor;
  std::string disableEffectParticles; // TODO ParticleSystem
  float sparksPerCubicFoot = 1.0f;
  std::set<WeaponAffection> unaffectedSides;
  bool spareLauncher = true; // EVAL redundancy?
  std::set<Attribute> targetInclusion;
  std::set<Attribute> targetExclusion;
};

struct EnemyNear : public Module {
  uint32_t scanDelayTime = 1;
};

struct FireOCLAfterWeaponCooldown : public Module {
  WeaponSlot slot;
  std::string creationList; // TODO ObjectCreationList
  uint32_t minShots = 1;
  uint32_t lifetimePerSecond = 1;
  uint32_t maxCap = 1;
};

struct FireSpread : public Module {
  std::string creationList; // TODO ObjectCreationList
  Duration minSpreadDelayMs = 0;
  Duration maxSpreadDelayMs = 0;
  float spreadRange = 0.0f;
};

struct FirestormDynamicGeometryInfo : public DynamicGeometryInfo {
  uint32_t delayBetweenFramesMs = 1000;
  float damage = 1.0f;
  float maxDamageHeight = 1.0f;
  std::string effect; // TODO FXList
  std::list<std::string> particles; // TODO ParticleSystem
  float particleHeightOffset = 0.0f;
  float scorchSize = 1.0f;
};

struct FireWeaponCollision : public Module {
  std::string weapon; // TODO Weapon
  bool once = false;
  std::set<Status> requiredStates;
  std::set<Status> excludedStates;
};

struct FireWeapon : public Module {
  std::string weapon;
  Duration initialDelayMs = 0;
  Duration exclusiveWeaponDelayMs = 0;
};

struct FireWeaponWhenDamaged : public Module {
  bool active = true;
  std::string continuousWeaponPristine;
  std::string continuousWeaponDamaged;
  std::string continuousWeaponReallyDamaged;
  std::string continuousWeaponRubble;
  std::set<DamageType> damageTypes;
  float damageAmount = 1.0f;
  std::string reactionWeaponPristine;
  std::string reactionWeaponDamaged;
  std::string reactionWeaponReallyDamaged;
  std::string reactionWeaponRubble;
};

struct FireWeaponWhenDead : public Module {
  bool active = true;
  std::string weapon; // TODO Weapon
};

struct Flammable : public Module {
  Duration burnedDelayMs = 0;
  Duration burningDurationMs = 0;
  Duration burningDamageDelayMs = 0;
  int32_t burningDamageAmount = 0;
  std::string sound;
  Damage damageLimit = 20.0f;
  Duration damageExpirationMs = 2; // TODO LF/S
};

struct FlightDeck : public AI {
  struct Runway {
    std::list<std::string> spaces;
    std::list<std::string> takeoff;
    std::list<std::string> landing;
    std::list<std::string> taxi;
    std::list<std::string> creation;
    std::string catapultParticles;
  };

  int32_t numRunways = 1;
  int32_t numSpacesPerRunway = 1;
  Runway runway1;
  Runway runway2;
  float approachHeight = 5.0f;
  float deckHeightOffset = 0.0f;
  float healingPerSecond = 1.0f;
  uint32_t parkingCleanupTime = 1;
  uint32_t humanFollowTime = 1;
  std::string payload;
  uint32_t replacementTime = 1;
  uint32_t dockAnimationTime = 1;
  uint32_t launchWaveTime = 1;
  uint32_t launchRampTime = 1;
  uint32_t lowerRampTime = 1;
  uint32_t catapultFireTime = 1;
};

struct Float : public Module {
  bool enabled;
};

struct GenerateMinefield : public Module {
  std::string mine; // object name
  std::string upgradedMine;
  std::string upgradeTrigger;
  std::string generationEffect;
  float distance = 5.0f;
  float minesPerSquareFoot = 1.0f;
  bool onlyOnDeath = false;
  bool borderOnly = false;
  bool smartBorder = false;
  bool smartBorderSkipInterior = false;
  bool circular = true;
  bool upgradable = false;
  float jitter = 1.0f;
  Percent skipWhenCovered = 50;
};

struct GrantStealth : public Module {
  float startRadius = 1.0f;
  float finalRadius = 1.0f;
  float growthRate = 1.0f;
  std::set<Attribute> affecting;
  std::string particles; // TODO ParticleSystem
};

struct GrantUpgrade : public Module {
  std::string upgrade; // TODO Upgrade
  std::set<Status> exclusions;
};

struct HackInternet : public AI {
  uint32_t unpackTime = 1;
  uint32_t packTime = 1;
  float packVariatonFactor = 0.0f;
  uint32_t cashUpdateDelay = 0;
  uint32_t cashUpdateDelayFast = 0;
  Money regularAmount = 1;
  Money veteranAmount = 2;
  Money eliteAmount = 3;
  Money heroicAmount = 4;
  uint32_t xpPerCashUpdate = 1;
};

struct HeightDie : public Module {
  float targetHeight = 1.0f;
  bool targetHeightForStructures = false;
  bool downwardsOnly = false;
  float destroyParticlesAt = 0.0f;
  bool toGroundOnDeath = true;
  uint32_t initialDelay = 0;
};

struct HiveStructureBody : public ActiveBody {
  std::set<DamageType> absorbDamages;
  std::set<DamageType> propagateDamages;
};

struct InstantDeath : public Die {
  std::list<std::string> effects; // TODO FXList
  std::list<std::string> creationLists; // TODO OCL
};

struct JetAI : public AI {
  float outOfAmmoDamagePerSecond = 1.0;
  bool needsRunway = true;
  bool keepsParkingSpace = true;
  Percent takeOffDist = 50;
  uint32_t takeOffPauseTime = 1;
  float minHeight = 1.0f;
  float parkingOffset = 0.0f;
  float sneakAttackOffset = 0.0f;
  LocomotorType attackLocomotion = LocomotorType::NORMAL;
  uint32_t atackPersistTime = 1.0f;
  LocomotorType returnLocomotion = LocomotorType::NORMAL;
  uint32_t lockOnTime = 1;
  std::string lockOnCursor;
  float lockOnInitialDist = 0.0f;
  float lockOnFreq = 1.0f;
  float lockOnAngleSpin = 0.0f;
  bool lockOnBlink = false;
  uint32_t idleReturnTime = 0;
};

struct Laser : public Module {
  std::string muzzleParticleSystem; // TODO ParticleSystem
  float punchThroughScalar = 1.0f;
  std::string targetParticleSystem; // ...
};

struct LeafletDrop : public Module {
  uint32_t delay = 0;
  uint32_t disabledDuration = 0;
  float radius;
  std::string leafletParticles; // TODO ParticleSystem
};

struct Lifetime : public Module {
  Duration minLifetimeMs;
  Duration maxLifetimeMs;
};

struct LockWeapon : public Module {
  WeaponSlot slot = WeaponSlot::NONE;
};

struct MissileAI : public AI {
  bool detonateOnFuelDepletion = false;
  bool detonationCallsKill = false;
  float distanceUntilDiving = 200.0f;
  float distanceUntilLock = 200.0f;
  float distanceUntilReturn = 200.0f;
  bool followTarget = true;
  Duration fuelLifetimeMs = 5000;
  uint32_t garrisonKillCount = 10;
  std::string garrisonKillEffect; // TODO FXList
  std::set<Attribute> garrisonKillExclusion;
  std::set<Attribute> garrisonKillInclusion;
  Duration ignitionDelayMs = 0;
  std::string ignitionEffect;
  float initialVelocity = 50.0f;
  Duration killSelfDelayMs = 1000;
  float jammedScatterDistance = 1.0f;
  bool weaponSpeed = false;
};

struct FactionOCL {
  std::string faction;
  std::string ocl;
};

struct OCL : public Module {
  bool createAtEdge = true;
  bool factionTriggered = true;
  std::list<FactionOCL> factionOCLs;
  std::string ocl; // OCL
  Duration maxDelayMs = 1000;
  Duration minDelayMs = 0;
};

struct OpenContain : public Module {
  bool allowAllies = true;
  bool allowNeutrals = false;
  bool allowEnemies = false;
  bool burnUnits = false;
  Percent damageToUnits = 0;
  Duration doorOpenTimeMs = 1000;
  std::string enterSound;
  std::string exitSound;
  std::set<Attribute> guestInclusion;
  std::set<Attribute> guestExclusion;
  int32_t max = 1;
  int32_t numExitPaths = 1;
  bool unitsCanFire = true;
  bool unitsInTurret = false;
  bool weaponBonusToUnits = false;
};

struct CaveContain : public OpenContain {
  int32_t caveIndex = 0;
};

struct GarrisonContain : public OpenContain {
  bool enclosing = true;
  float fullHealTime = 1.0f;
  bool heal = false;
  std::string initialRoster; // object name
  bool mobile = false;
  uint32_t numInitial = 1;
  bool noRaidAttack = false;
};

struct HealContain : public OpenContain {
  Duration timeToFullHealthMs = 5000;
};

struct ParachuteContain : public OpenContain {
  float freeFallDamage = 0.0f; // percent
  float lowAltDampening = 0.0f;
  std::string openingSound;
  float pitchRateMax = 1.0f;
  float rollRateMax = 1.0f;
  float travelToOpenDist = 10.0f;
};

struct TransportContain : public OpenContain {
  bool armedRidersWeaponUpgrade = true;
  bool destroyTrappedRiders = true;
  bool exitAggressively = true;
  std::string exitBone;
  Duration exitDelayMs = 1000;
  bool exitDelayInAir = true;
  float exitPitchRate = 1.0f;
  bool exitContainerKeepSpeed = false;
  bool exitOrientationAsContainer = false;
  bool exitResetMoodCheck = true;
  bool exitScattering = true;
  std::string initialPayload; // objects?
  float healthRegenPerSecond = 1.0f;
  uint32_t slots = 1;
};

struct HelixContain : public TransportContain {
  bool drawPips = true;
  std::list<std::string> templates;
};

struct OverlordContain : public TransportContain {
  std::string payload;
  bool xpForRider = false;
};

struct ParkingPlace : public Module {
  float approachHeight = 50.0f;
  bool hasRunways = true;
  int32_t numCols = 1;
  int32_t numRows = 1;
  float healingPerSecond = 1.0f;
};

struct Physics : public Module {
  float aerodynamicFriction = 0.0f; // percent?
  bool bouncing = false;
  bool collisionForce = true;
  float fallHeightDamageFactor = 1.0f;
  float factor; // EVAL pitchRollYawFactor
  float forwardFriction = 0.0f; // percent?
  glm::vec3 friction = {0.15f, 0.8f, 0.15f};
  bool killOnGround = false;
  float mass = 1.0f;
  float massCenterOffset = 0.0f;
  float minFallSpeedForDamage = 2.0f; // TODO lookup calc
  float shockResistance = 0.0f;
  glm::vec3 shockMax = {0.05f, 0.025f, 0.025f};
};

struct PilotFindVehicle : public Module {
};

struct PointDefenseLaser : public Module {
  std::set<Attribute> primaryTargets;
  std::set<Attribute> secondaryTargets;
  uint32_t scanRate = 1;
  float scanRange = 10.0f;
  float velocityFactor = 1.0f;
  std::string weapon;
};

struct Poisoned : public Module {
  Duration intervalMs = 1000;
  Duration durationMs = 3000;
};

struct SalvageCrateCollision : public CrateCollision {
  Percent weaponChance = 0;
  Percent levelChance = 0;
  Percent moneyChance = 0;
  Money minMoney = 0;
  Money maxMoney = 1;
};

enum class SlowDeathPhase {
    INITIAL
  , MIDPOINT
  , FINAL
};

struct SlowDeathCreationList {
  SlowDeathPhase phase = SlowDeathPhase::INITIAL;
  std::string creationList; // TODO ObjectCreationList
};

struct SlowDeathEffect {
  SlowDeathPhase phase = SlowDeathPhase::INITIAL;
  std::string effect; // TODO FXList
};

struct SlowDeath : public Die {
  float sinkRate = 0.0f;
  Duration sinkDelayMs = 0;
  Duration sinkDelayVarianceMs = 0;
  int32_t probability = 10;
  Percent modBonusPerOverkill = 0;
  std::set<DeathType> deathTypes;
  Duration destructionDelayMs = 0;
  Duration destructionDelayVarianceMs = 0;
  float desctructionAlt = -10.0f;
  std::list<SlowDeathEffect> effects;
  std::list<SlowDeathCreationList> creationLists;
  std::list<SlowDeathEffect> weapons;
  float flingForce = 0.0f;
  float flingForceVariance = 0.0f;
  float flingPitch = 0.0f;
  float flingPitchVariance = 0.0f;
};

struct BattleBusSlowDeath : public SlowDeath {
  std::string effectHitGround;
  std::string effectStartUndeath;
  std::string creationListStartUndeath;
  std::string creationListHitGround;
  float throwForce = 1.0f;
  Percent damageToPassengers = 0;
  uint32_t emptyDestructionDelay = 0;
};

// EVAL delay docs vary between frames and ms
struct HelicopterSlowDeath : public SlowDeath {
  std::string bladeEffect; // FX
  std::string bladeCreationList; // OCL
  std::string blades;
  std::string bladesBone;
  std::string deathSound;
  Duration delayToBlowupMs = 2000;
  std::string ejectPilotEffect; // FX
  std::string ejectPilotCreationList; // OCL
  Percent fallSpeed = 100;
  std::string finalBlowupEffect; // FX
  std::string finalBlowupCreationList; // OCL
  std::string hitGroundEffect; // FX
  std::string hitGroundCreationList; // OCL
  Duration maxBladeFallOffDelayMs = 1000;
  float maxBraking = 100.0f;
  float maxSpin = 100.0f;
  Duration minBladeFallOffDelayMs = 1000;
  float minSpin = 100.0f;
  std::string particles;
  std::string particlesBone;
  glm::vec3 particlesLocation;
  std::string rubble; // Object
  float spinUpdateAmount = 100.0f;
  float spinUpdateDelayFrames = 100.0f;
  float spiralTurnRate = 100.0f;
  float spiralForwardSpeed = 100.0f;
  float spiralForwardSpeedDampening = 0.0f;
};

// EVAL delay docs vary between frames and ms
struct JetSlowDeath : public SlowDeath {
  std::string deathSound;
  Duration delayToSecondaryDeathMs = 2000;
  Duration delayToBlowupMs = 2000;
  Percent fallSpeed = 100;
  std::string finalBlowupEffect; // FX
  std::string finalBlowupCreationList; // OCL
  std::string groundDeathEffect; // FX
  std::string groundDeathCreationList; // OCL
  std::string hitGroundEffect; // FX
  std::string hitGroundCreationList; // OCL
  std::string initialDeathEffect; // FX
  std::string initialDeathCreationList; // OCL
  float pitchRate = 0.0f;
  float rollRate = 0.0f;
  Percent rollRateDelta = 100;
  std::string secondaryEffect; // FX
  std::string secondaryCreationList; // OCL
};

struct NeutronMissileSlowDeath : public SlowDeath {
  struct Blast {
    bool enabled = true;
    float delay = 0.0f;
    float scorchDelay = 1.0f;
    float innerRadius = 1.0f;
    float outerRadius = 2.0f;
    float minDamage = 0.0f;
    float maxDamage = 0.0f;
    float toppleSpeed = 1.0f;
    float pushForce = 0.0f;
  };

  float scorchMarkSize = 10.0f;
  std::string effect; // TODO FXList
  std::array<Blast, 8> blasts;
};

// TODO
struct ParticleUplinkCannon : public Module {
};

struct PowerPlant : public Module {
  Duration rodsExtendTimeMs = 1000;
};

struct Production : public Module {
  Duration constructionTimeMs = 1000;
  Duration doorClosingTimeMs = 1000;
  Duration doorOpeningTimeMs = 1000;
  Duration doorWaitOpenTimeMs = 1000;
  int32_t maxQueue = 10;
  int32_t numDoorAnimations = 1;
};

struct Radar : public Module {
  Duration extendTimeMs = 1000;
};

struct RepairDock : public Dock {
  Duration timeToHealMs = 5000;
};

struct SpecialPower : public Module {
  std::string specialPower; // TODO Special power
  bool updateStartsAttack = false;
  bool paused = false;
  std::string sound;
  bool scriptedOnly = false;
};

struct SpecialPowerUpdate : public Module {
  float abilityAbortRange = 1.0f;
  float abilityStartRange = 1.0f;
  std::string captureEffect;
  float fleeRange = 100.0f;
  bool loseStealth = true;
  uint16_t maxSpecialObjects = 1;
  Duration packTimeMs = 1000;
  Duration preparationTimeMs = 0;
  Duration persistentPreparationTimeMs = 0;
  bool skipPackingWithoutTarget = true;
  std::string specialPower; // TODO Special power
  std::string specialObject; // Object
  bool specialObjectPersists = true;
  bool specialObjectPersistsOnDeath = true;
  std::string specialObjectToBone;
  bool specialObjectUniquePerTarget = true;
  bool switchOwnerAfterUnpacking = false;
  std::string unpackSound;
  Duration unpackTimeMs = 1000;
  Duration unstealthTimeMs = 1000;
  bool validateSpecialObject = false;
  uint32_t xpAward = 0;
};

struct BaikonurLaunchPower : public SpecialPower {
  std::string detonationObject;
};

struct CashBounty : public SpecialPower {
  Percent bounty = 1;
};

struct CashHackUpgrade {
  std::string upgrade; // TODO Upgrade
  Money amount;
};

struct CashHack : public SpecialPower {
  std::list<CashHackUpgrade> upgrades;
  Money amount;
};

struct CleanupArea : public SpecialPower {
  float maxMoveDistance = 1.0f;
};

struct FireWeaponPower : public SpecialPower {
  uint32_t maxShots = 1;
};

struct OCLSpecialPower : public SpecialPower {
  bool adjustToNextPassable = true;
  OCLLocation location = OCLLocation::NEAR_SOURCE;
  std::list<std::string> OCLs; // TODO OCL
  std::string reference;
  std::list<std::pair<std::string, std::string>> upgradeOCLs;
};

struct SpyVisionSpecialPower : public SpecialPower {
  Duration baseDurationMs = 1000;
  Duration bonusDurationPerCaptureMs = 1000;
  Duration maxDurationMs = 10000;
};

struct Spawn : public Module {
  bool aggregateHealth = false;
  int32_t initialBurst = 0;
  int32_t number = 1;
  bool once = false;
  bool reclaimOrphans = false;
  Duration replaceDelayMs = 1000;
  bool requiresSpawner = false;
  std::set<DamageType> propagatedDamageTypes;
  std::string spawn;
  bool spawnsWithFreeWill = true;
};

struct Slaved : public Module {
  int32_t attackRange = 100;
  int32_t attackWanderRange = 100;
  int32_t guardMaxRange = 100;
  int32_t guardWanderRange = 100;
  Duration readyMaxMs = 1000;
  Duration readyMinMs = 0;
  float repairMaxAlt = 10.0f;
  float repairMinAlt = 0.0f;
  int32_t repairRange = 10;
  float repairRate = 1.0f;
  Percent repairWhenHealthBelow = 50;
  bool sameLayer = false;
  int32_t scoutRange = 100;
  int32_t scoutWanderRange = 100;
  int32_t targetMasterBonusRange = 10;
  Duration weldMaxMs = 1000;
  Duration weldMinMs = 0;
  std::string welding;
  std::string weldingEffectBone;
};

// TODO
struct SpectreGunship : public Module {
};

// TODO
struct SpectreGunshipDeployment : public Module {
};

struct Stealth : public Module {
  Duration delayMs = 500;
  std::set<Status> detectableStates;
  std::string enemyDetectionEvaEvent; // EvaEvent
  std::set<StealthLevel> forbiddenConditions;
  Duration pulseFrequencyMs = 500;
  Percent friendlyOpacityMin = 50;
  Percent friendlyOpacityMax = 100;
  bool gettingAttackWhenRevealed = true;
  bool innateStealth = true;
  float moveSpeedThreshold = 3.0f;
  std::string ownDetectionEvaEvent; // EvaEvent
};

struct StructureTopple : public Module {
  std::string creationList;
  std::string crushingEffect;
  std::string crushingWeapon;
  std::set<DamageType> damageTypes;
  float decay = 0.0f;
  float effectAngle = 0.0f;
  std::string angleEffectName;
  float integrity = 1.0f;
  Duration maxToppleBurstDelayMs = 1000;
  Duration maxToppleDelayMs = 1000;
  Duration minToppleBurstDelayMs = 0;
  Duration minToppleDelayMs = 0;
  std::string topplingEffect;
  std::string topplingDelayEffect;
  std::string topplingDoneEffect;
  std::string topplingStartEffect;
};

struct SupplyCenterDock : public Dock {
  Duration temporaryStealthMs = 5000;
};

struct SupplyTruckAI : public AI {
  int32_t maxBoxes = 3;
  uint32_t supplyCenterDelay = 3;
  uint32_t warehouseDelay = 3;
  float warehouseScanDistance = 10.0f;
  std::string depletedSound;
};

struct SupplyWarehouseCrippling : public Module {
  Health healAmount = 10.0f;
  Duration healIntervalMs = 1000;
  Duration healSuppressionMs = 1000;
};

struct SupplyWarehouseDock : public Dock {
  bool deleteWhenEmpty = false;
  int32_t numBoxes = 100;
};

struct ChinookAI : public SupplyTruckAI {
  float ropeDropSpeed = 2.0f;
  float rappelSpeed = 2.0f;
  std::string ropeName; // object name?
  float ropeHeight = 1.0f;
  float ropeWidth = 1.0f;
  float ropeWobbleLenght = 1.0f;
  float ropeWobbleAmplitude = 1.0f;
  float ropeWobbleRate = 1.0f;
  Color ropeColor;
  uint32_t numRopes = 4;
  uint32_t minRopeDelay = 0;
  uint32_t maxRopeDelay = 1;
  float dropHeight = 1.0f;
  bool waitForRopeDrop = true;
  std::string rotorWashParticles; // TODO ParticleSystem
  int32_t supplyBoost = 0;
};

struct StealthDetector : public Module {
  bool detectWhenGarrisoned = false;
  bool detectWhenContained = false;
  std::set<Attribute> detectionInclusion;
  std::set<Attribute> detectionExclusion;
  bool disabled = false;
  std::string bone; // TODO ?!
  std::string pingSound;
  std::string loudPingSound;
  std::string beaconParticles; // TODO ParticleSystem
  std::string particles;
  std::string brightParticles;
  std::string gridParticles;
  Duration rateMs = 1000;
  float range = 5.0f;
};

struct CollapseEvent {
  StructureCollapsePhase phase;
  std::string event;
};

struct StructureCollapse : public Module {
  int32_t bigBurstFrequency = 1;
  float collapseDampening = 0.0f;
  Duration maxBurstDelayMs = 1000;
  Duration maxCollapseDelayMs = 1000;
  float maxShudder = 0.0f;
  Duration minBurstDelayMs = 0;
  Duration minCollapseDelayMs = 0;
  std::list<CollapseEvent> creationLists; // OCL
  std::list<CollapseEvent> effects; // FXList
};

struct TechBuilding : public Module {
  std::list<std::string> effects; // TODO FXList
  Duration rateMs = 1000;
};

struct Topple : public Module {
  std::string toppleEffect; // TODO FXList
  std::string bounceEffect;
  std::string stump; // object name?
  bool killOnStartToppling = false;
  bool killOnStopToppling = true;
  bool killStumpWhenToppled = false;
  bool reorientToppledRubble = false;
  bool oneAxisOnly = false;
  Percent initialAcceleration = 1;
  Percent bounceVelocity = 30;
};

struct TransitionDamageTypeFX {
  glm::vec3 location;
  std::string effect; // TODO FXList
};

struct TransitionDamageTypeParticles {
  std::string bone;
  std::string particleSystem;
  bool randomBone;
};

using TransitionDamageFXSlots = std::array<std::array<TransitionDamageTypeFX, 12>, 4>;
using TransitionDamageParticlesSlots = std::array<std::array<TransitionDamageTypeParticles, 12>, 4>;

struct TransitionDamageFX : public Module {
  std::set<DamageType> damageEffectTypes;
  TransitionDamageFXSlots effects;
  std::set<DamageType> damageOCLTypes;
  TransitionDamageFXSlots OCLs;
  std::set<DamageType> damageParticleTypes;
  TransitionDamageParticlesSlots particleSystems;
};

struct Upgrade : public Module {
  std::list<std::string> triggers; // TODO Upgrade
  std::list<std::string> conflicts;
  std::list<std::string> removing;
  bool needAllTriggers = false;
};

struct CommandSetUpgrade : public Upgrade {
  std::string commandSet1; // TODO CommandSet
  std::string commandSet2; // TODO CommandSet
  std::string trigger; // TODO Upgrade
};

struct CostModifierUpgrade : public Upgrade {
  std::set<Attribute> affecting; // EVAL
  Percent percentage = 0;
};

struct ExperienceScalarUpgrade : public Upgrade {
  float xpScalar = 1.0f;
};

struct FireWeaponWhenDamagedUpgrade : public Upgrade {
  bool active = true;
  std::string reactionWeaponPristine;
  std::string reactionWeaponDamaged;
  std::string reactionWeaponSeverlyDamaged;
  std::string reactionWeaponBroken;
  std::string continuousWeaponPristine;
  std::string continuousWeaponDamaged;
  std::string continuousWeaponSeverlyDamaged;
  std::string continuousWeaponBroken;
  std::set<DamageType> damageTypes;
  float damageAmount = 1.0f;
};

struct FireWeaponWhenDeadUpgrade : public Die, public Upgrade {
  bool active = true;
  std::string deathWeapon; // TODO Weapon
};

struct GrantScienceUpgrade : public Upgrade {
  std::string science; // TODO Science
};

struct MaxHealthUpgrade : public Upgrade {
  Health healthUpgrade = 10.0f;
  MaxHealthModifier modifier;
};

struct ObjectCreationUpgrade : public Upgrade {
  std::string object;
};

struct UnpauseSpecialPowerUpgrade : public Upgrade {
  std::string specialPower; // TODO special power
};

struct UpgradeDie : public Die {
  std::string removeUpgrade; // TODO Upgrade
};

struct VeterancyCrateCollision : public CrateCollision {
  uint32_t effectRange = 10;
  bool isPilot = false;
  bool veterancyToTarget = false;
};

struct VeterancyGain : public Module {
  Veterancy starting = Veterancy::REGULAR;
  std::string requiredScience; // TODO Science
};

struct WeaponBonus : public Module {
  std::set<Attribute> affectedInclusion;
  std::set<Attribute> affectedExclusion;
  uint32_t duration = 1;
  uint32_t delay = 1;
  float range = 5.0f;
  WeaponBonusCondition condition;
};

struct WaveGuide : public Module {
  float delay = 0.0f;
  float ySize = 1.0f; // EVAL axis?
  float linearSpacing = 1.0f;
  float bendMagnitude = 1.0f;
  float velocity = 1.0f;
  float preferredHeight = 0.0f;
  float shorelineEffectDistance = 0.0f;
  float damageRadius = 0.0f;
  float damageAmount = 1.0f;
  float toppleForce = 0.0f;
  std::string splashSound;
  int32_t splashSoundRate = 1;
  std::string bridgeParticles; // TODO ParticlesSystem
  float bridgeParticlesAngle = 0.0f;
  std::string loopingSound;
};

}

#endif
