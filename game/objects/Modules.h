#ifndef H_GAME_OBJECT_MODULE
#define H_GAME_OBJECT_MODULE

#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <set>
#include <string>

#include <glm/glm.hpp>

#include "Attributes.h"
#include "../Color.h"

namespace ZH::Objects {

struct Module {
};

enum class ModuleType {
    ACTIVE_BODY
  , AI
  , ANIMATION_STEERING
  , ASSAULT_TRANSPORT
  , ASSISTED_TARGETING
  , AUTO_DEPOSIT
  , AUTO_FIND_HEALING
  , AUTO_HEAL
  , BAIKONUR_LAUNCH_POWER
  , BATTLE_BUS_SLOW_DEATH
  , BATTLE_PLAN
  , BONE_FX
  , BRIDGE
  , BUNKER_BUSTER
  , BODY
  , CASH_BOUNTY
  , CASH_HACK
  , CAVE_CONTAIN
  , CHECKPOINT
  , CHINOOK
  , CLEANUP_AREA
  , CLEANUP_HAZARD
  , COMMAND_BUTTON_HUNT
  , COMMAND_SET_UPGRADE
  , CONVERT_TO_CAR_BOMB
  , CONVERT_TO_HIJACK
  , COST_MODIFIER
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
  , DEPLOY_STYLE
  , DIE
  , DOCK
  , DOZER
  , DUMB_PROJECTILE
  , DYNAMIC_GEOMETRY_INFO
  , DYNAMIC_SHROUD_CLEARING_RANGE
  , EJECT_PILOT_DIE
  , EMP
  , ENEMY_NEAR
  , EXPERINCE_SCALAR
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
  , GRANT_SCIENCE
  , GRANT_STEALTH
  , GRANT_UPGRADE
  , HACK_INTERNET
  , HEAL_CONTAIN
  , HEIGHT_DIE // cont here
  , HIGHLANDER_BODY
  , JET
  , LEAFLET_DROP
  , LIFETIME
  , MONEY_CRATE_COLLISION
  , NEUTRON_MISSILE_SLOW_DEATH
  , OPEN_CONTAIN
  , PHYSICS
  , SALVAGE_CRATE_COLLISION
  , SPECIAL_POWER
  , SPECIAL_POWER_COMPLETION_DIE
  , SLOW_DEATH
  , SUPPLY_TRUCK
  , STEALTH_DETECTOR
  , SWAY_CLIENT
  , TOPPLE
  , UPGRADE
  , WAVE_GUIDE
  , WEAPON_BONUS
};

struct Turret {
  bool canPitch = true;
  bool canFireAndTurn = true;
  bool disabled = false;
  WeaponSlot controlledSlot = WeaponSlot::PRIMARY;
  std::optional<float> fixedFirePitch;
  std::array<std::optional<float>, 3> fireAngleSweep;
  float groundUnitPitch = 0.0f;
  float minPitch = 0.0f;
  float minScanAngle = 0.0f;
  uint32_t minScanInterval = 1;
  float maxScanAngle = 0.0f;
  uint32_t maxScanInterval = 1;
  uint32_t recenterTimeMs = 1;
  glm::vec2 rotationRate;
  glm::vec2 rotation;
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
  bool acquireEnemiesWhenIdle = true;
  bool ignorePlayer = false;
  uint32_t moodAttackCheckRate = 1;
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

struct AssistetedTargeting : public Module {
  uint32_t numShots = 1;
  WeaponSlot slot;
  std::string laserFrom; // object names
  std::string laserTo;
};

struct AutoDeposit : public Module {
  uint32_t depositTimingMs;
  Money depositAmount;
  Money captureBonus;
  bool actualMoney;
  std::string upgradeToBoost; // TODO Upgrade
  uint8_t boostValue = 1;
};

struct AutoFindHealing : public Module {
  uint32_t scanRateMs = 1000;
  float scanRange = 10.0f;
  float neverHeal = 100.0f;
  float alwaysHeal = 1.0f;
};

struct AutoHeal : public Module {
  bool enabled = true;
  bool singleBurst = false;
  int32_t healingAmount = 1;
  uint32_t healingDelay = 0;
  float radius = 10.0f;
  std::set<Attribute> healingInclusion;
  std::set<Attribute> healingExclusion;
  std::string radiusParticleSystem; // TODO ParticleSystem
  std::string healParticleSyste;
  uint32_t startHealingDelay = 0;
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
  HealthModifier hdlBuildingHealthModifier = HealthModifier::PRESERVE_RATIO;
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

struct BoneFX : public Module {
  // EVAL Investigate further
};

struct BridgeDieEffect {
  std::string effect; // TODO FXList
  uint32_t delayMs = 1000;
  std::string bone; // TODO ?!
};

struct BridgeDieConstruction {
  std::string creationList; // TODO ObjectCreationList
  uint32_t delayMs = 1000;
  std::string bone; // TODO ?!
};

struct Bridge : public Module {
  float lateralScaffoldSpeed = 1.0f;
  float verticalScaffoldSpeed = 1.0f;
  std::list<BridgeDieEffect> dieEffects;
  std::list<BridgeDieConstruction> dieConstructions;
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
  // EVAL Death types
  // EVAL veteran levels
  std::set<Status> requiredStates;
  std::set<Status> excludedStates;
};

struct CreateCreateDie : public Die {
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
  glm::vec3 defaultPoint;
  glm::vec3 rallyPoint;
  bool useRallyPoint = false;
};

struct Deletion : public Module {
  uint32_t minLifetime;
  uint32_t maxLifetime;
};

struct DeliverPayload : public AI {
  uint32_t doorDelay = 1;
  std::string dropCarrier; // object name
  float distance = 1.0f;
  int32_t numAttempts = 3;
  uint32_t dropDelay = 1;
  glm::vec3 offset;
  glm::vec3 variance;
  std::string decal; // TODO Decal
  float decalRadius = 2.0f;
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

struct DeployStyle : public Module {
  uint32_t unpackTime = 1;
  uint32_t packTime = 1;
  bool resetBeforePacking = true;
  bool deployRequired = true;
  bool centerBeforePacking = true;
  bool manualDeployAnimation = false;
};

struct Dock : public Module {
  int32_t numApproachPositions = 1;
  bool allowPassThrough = false;
};

struct Dozer : public AI {
  Percent repairPerSecond;
  float boredTime = 5.0f;
  float boredRange = 5.0f;
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
  uint32_t initialDelay = 0;
  uint32_t exclusiveWeaponDelay = 0;
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

struct LeafletDrop : public Module {
  uint32_t delay = 0;
  uint32_t disabledDuration = 0;
  float radius;
  std::string leafletParticles; // TODO ParticleSystem
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
  bool onlyDownwards = false;
  bool destroyParticles = false;
  bool toGroundOnDeath = true;
  uint32_t initialDelay = 0;
};

struct Jet : public AI {
  float outOfAmmoDamagePerSecond = 1.0;
  bool needsRunway = true;
  bool keepsParkingSpace = true;
  Percent takeOffDist = 50;
  uint32_t takeOffPauseTime = 1;
  float minHeight = 1.0f;
  float parkingOffset = 0.0f;
  float sneakAttackOffset = 0.0f;
  Locomotor attackLocomotion = Locomotor::NORMAL;
  uint32_t atackPersistTime = 1.0f;
  Locomotor returnLocomotion = Locomotor::NORMAL;
  uint32_t lockOnTime = 1;
  std::string lockOnCursor;
  float lockOnInitialDist = 0.0f;
  float lockOnFreq = 1.0f;
  float lockOnAngleSpin = 0.0f;
  bool lockOnBlink = false;
  uint32_t idleReturnTime = 0;
};

struct Lifetime : public Module {
  Duration minLifetimeMs;
  Duration maxLifetimeMs;
};

struct OpenContain : public Module {
  int32_t max = 1;
  std::string enterSound;
  std::string exitSound;
  Percent damageToUnits = 0;
  bool burnUnits = false;
  std::set<Attribute> guestInclusion;
  std::set<Attribute> guestExclusion;
  bool unitsCanFire = true;
  bool unitsInTurret = false;
  int32_t numExitPaths = 1;
  uint32_t doorOpenTime = 1;
  bool weaponBonusToUnits = false;
  bool allowAllies = true;
  bool allowNeutrals = false;
  bool allowEnemies = false;
};

struct CaveContain : public OpenContain {
  int32_t caveIndex = 0;
};

struct GarrisonContain : public OpenContain {
  bool mobile = false;
  bool heal = false;
  float fullHealTime = 1.0f;
  std::string initialRoster; // object name
  uint32_t numInitial = 1;
  bool noRaidAttack = false;
  bool enclosing = true;
};

struct HealContain : public OpenContain {
  uint32_t timeToFullHealth = 1;
};

struct Physics : public Module {
  float mass = 1.0f;
  float geometryMassCenterOffset = 0.0f;
  float shockResistance = 0.0f;
  glm::vec3 shockMax = {0.05f, 0.025f, 0.025f};
  glm::vec3 friction = {0.15f, 0.8f, 0.15f};
  float aerodynamicFriction = 0.0f;
  bool bouncing = false;
  bool killOnGround = false;
  bool collisionForce = true;
  float minFallSpeedForDamage = 2.0f; // TODO lookup calc
  float fallHeightDamageFactor = 1.0f;
  float factor; // EVAL pitchRollYawFactor
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
  std::string objectCreationList; // TODO ObjectCreationList
};

struct SlowDeathEffect {
  SlowDeathPhase phase = SlowDeathPhase::INITIAL;
  std::string effect; // TODO FXList
};

struct SlowDeath : public Module {
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
  std::string weapon; // TODO Weapon
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

struct SpecialPower : public Module {
  std::string specialPower;
  bool updateStartsAttack = false;
  bool paused = false;
  std::string sound;
  bool scriptedSpecial = false;
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

struct CostModifier : public SpecialPower {
  std::set<Attribute> affecting; // EVAL
  Percent percentage = 1;
};

struct FireWeaponPower : public SpecialPower {
  uint32_t maxShots = 1;
};

struct SupplyTruck : public AI {
  int32_t maxBoxes = 3;
  uint32_t supplyCenterDelay = 3;
  uint32_t warehouseDelay = 3;
  float warehouseScanDistance = 10.0f;
  std::string depletedSound;
};

struct Chinook : public SupplyTruck {
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
  uint32_t rate = 1;
  float range = 5.0f;
  bool disabled = false;
  std::string pingSound;
  std::string loudPingSound;
  std::string beaconParticles; // TODO ParticleSystem
  std::string particles;
  std::string brightParticles;
  std::string gridParticles;
  std::string bone; // TODO ?!
  std::set<Attribute> detectionInclusion;
  std::set<Attribute> detectionExclusion;
  bool detectWhenGarrisoned = false;
  bool detectWhenContained = false;
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

struct ExperienceScalar : public Upgrade {
  float xpScalar = 1.0f;
};

struct FireWeaponWhenDamaged : public Upgrade {
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

struct FireWeaponWhenDead : public Die, public Upgrade {
  bool active = true;
  std::string deathWeapon; // TODO Weapon
};

struct GrantScience : public Upgrade {
  std::string science;
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
