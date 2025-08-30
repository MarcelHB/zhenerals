#ifndef H_GAME_OBJECT_DRAWING
#define H_GAME_OBJECT_DRAWING

#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "../common.h"
#include "Attributes.h"

namespace ZH::Objects {

struct DrawData {
};

enum class DrawType {
    LASER_DRAW
  , DEPENDENCY_MODEL_DRAW
  , MODEL_DRAW
  , OVERLORD_AIRCRAFT_DRAW
  , OVERLORD_TRUCK_DRAW
  , TANK_DRAW
  , TREE_DRAW
  , TRUCK_DRAW
  , SUPPLY_DRAW
};

struct WeaponFX {
  WeaponSlot slot = WeaponSlot::NONE;
  std::string effect;
};

struct Animation {
  std::string name;
  uint32_t distanceCovered = 0;
  uint32_t interval = 0;
};

struct ConditionState {
  struct Turret {
    std::string name; // Bone
    float artAngle = 0.0f;
    std::string pitchName; // Bone
    float artPitch = 0.0f;
  };
  std::shared_ptr<Animation> animation;
  AnimationMode animationMode = AnimationMode::NONE;
  std::set<ModelCondition> conditions;
  std::set<AnimationFrameMode> flags;
  std::vector<std::string> hiddenSubObjects;
  std::list<Animation> idleAnimations;
  bool isTransition = false;
  float minAnimationSpeed = 1.0f; // one field
  float maxAnimationSpeed = 1.0f;
  std::string model;
  std::list<std::vector<std::string>> particleBones;
  std::vector<std::string> shownSubObjects;
  std::vector<std::string> tags;
  std::pair<std::string, std::string> transitionFromTo;
  std::string transitionKey;
  Turret turret1;
  Turret turret2;
  std::string waitForState;
  std::list<WeaponFX> weaponFireEffectBones;
  std::list<WeaponFX> weaponHideShowBones;
  std::list<WeaponFX> weaponLaunchBones;
  std::list<WeaponFX> weaponMuzzleFlashBones;
  std::list<WeaponFX> weaponRecoilBones;
};

struct LaserDrawData : public DrawData {
  float arcHeight = 30.0f;
  Duration fadeLifetimeMs = 1000;
  float innerBeamWidth = 1.0f;
  Color innerColor;
  bool isTile = true;
  Duration maxItensityLifetimeMs = 2000;
  uint32_t numBeams = 1;
  float outerBeamWidth = 1.0f;
  Color outerColor;
  float scrollRate = 1.0f;
  uint32_t segments = 1;
  float segmentsOverlapRatio = 0.0f;
  std::string texture;
  float tilingScalar = 1.0f;
};

struct ModelDrawData : public DrawData {
  bool animationRequiresPower = true;
  bool animatedParticles = true;
  bool canChangeColor = false;
  std::list<ConditionState> conditionStates;
  ConditionState defaultConditionState;
  bool dynamicIllumination = true;
  std::list<std::string> extraBones;
  std::string externalBoneAttachment;
  std::set<WeaponSlot> feedbackSlots;
  std::set<ModelCondition> ignoreConditions;
  float initialRecoilSpeed = 2.0f;
  float maxRecoilDistance = 3.0f;
  float recoilDamping = 0.4f;
  float recoilSettleSpeed = 0.065f;
  std::list<std::vector<ModelCondition>> stateAliases; // EVAL structure
  std::string trackMarksTexture;
  std::list<ConditionState> transitionStates;
};

struct DependencyModelDrawData : public ModelDrawData {
  std::string attachTo;
};

struct TankDrawData : public ModelDrawData {
  float treadAnimationRate = 1.0f;
  std::string treadDebrisLeft;
  std::string treadDebrisRight;
};

struct TreeDrawData : public DrawData {
  std::string model;
  Duration moveInwardTimeMs = 1000;
  Duration moveOutwardTimeMs = 1000;
  float moveOutwardDistanceFactor = 1.0f;
  std::string texture;
  std::string toppleEffect; // TODO FXList
  std::string bounceEffect;
  std::string stump; // object name?
  bool killOnStopToppling = true;
  bool topple = false;
  Percent initialAcceleration = 1;
  Percent initialVelocity = 20;
  Percent bounceVelocity = 30;
  float minToppleSpeed = 0.5f;
  float sinkDistance = 20.0f;
  Duration sinkTimeMs = 1000;
  bool shadow = false;
};

struct TruckDrawData : public ModelDrawData {
  std::string cabBone;
  float cabRotationFactor = 1.0f;
  std::string dirtEffect;
  std::string dustEffect;
  std::string powerslideEffect;
  float powerslideRotationAddition = 0.0f;
  float rotationDampeningFactor = 0.0f;
  float rotationSpeedMul = 1.0f;
  // FL, FR, RL, RR, MFL, MFR, MRL, MRR, MML, MMR
  std::array<std::string, 10> tireBones;
  std::string trailerBone;
  float trailerRotationFactor = 1.0f;
};

struct SupplyDrawData : public ModelDrawData {
  std::string supplyBonePrefix;
};

}

#endif
