#ifndef H_GAME_OBJECT_DRAWING
#define H_GAME_OBJECT_DRAWING

#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "Attributes.h"

namespace ZH::Objects {

struct DrawData {
};

enum class DrawType {
    MODEL_DRAW
  , TREE_DRAW
};

struct ConditionState {
  struct Turret {
    std::string name;
    float artAngle = 0.0f;
    std::string pitchName;
    float artPitch = 0.0f;
  };
  bool isTransition = false;
  std::set<ModelCondition> conditions;
  std::set<std::string> transitionKeys;
  std::string model;
  Turret turret1;
  Turret turret2;
  std::list<std::string> shownSubObjects;
  std::list<std::string> hiddenSubObjects;
  std::string weaponRireEffectBone;
  std::string weaponRecoilBone;
  std::string weaponMuzzleFlashBone;
  std::string weaponLaunchBone;
  std::string animation;
  std::string idleAnimation;
  AnimationMode animationMode = AnimationMode::ONCE;
  std::string transitionKey;
  std::set<AnimationFrameMode> flags;
  std::list<std::vector<std::string>> particleBones;
  float minAnimationSpeed = 1.0f; // one field
  float maxAnimationSpeed = 1.0f;
  std::vector<std::string> tags;
};

struct ModelDrawData : public DrawData {
  float initialRecoilSpeed = 2.0f;
  float maxRecoilDistance = 3.0f;
  float recoilDamping = 0.4f;
  float recoildSettleSpeed = 0.065f;
  bool canChangeColor = false;
  bool animationRequiresPower = true;
  bool animatedParticles = true;
  std::set<WeaponSlot> feedbackSlots;
  ConditionState defaultConditionState;
  std::list<ConditionState> conditionStates;
  std::list<std::list<ModelCondition>> stateAliases; // EVAL structure
  std::list<ConditionState> transitionStates;
  std::string trackMarksTexture;
  std::string extraBone; // TODO ?!
  std::string externalBoneAttachment;
  std::set<ModelCondition> ignoredConditions;
  bool dynamicIllumination = true;
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

}

#endif
