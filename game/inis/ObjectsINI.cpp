// SPDX-License-Identifier: GPL-2.0

#include "ObjectsINI.h"
#include "../Logging.h"
#include "../MurmurHash.h"

// Workarounds as linking to `(&)Objects::get...` stopped working at some binary sizes
// needs some more investigation eventually
#define CALL(x) \
  [](const std::string_view& v) { return x (v); }
#define SKIP(T) \
  [](T& t, INIFile& f) { return Skip(t, f); }

namespace ZH {

static size_t weaponSlotToIdx(Objects::WeaponSlot slot) {
  switch (slot) {
    case Objects::WeaponSlot::PRIMARY: return 0;
    case Objects::WeaponSlot::SECONDARY: return 1;
    default: return 2;
  }
}

template<typename T>
bool Skip(T&, INIFile& f) {
  f.parseString();
  return true;
}

static INIApplierMap<Objects::ActiveBody> ActiveBodyKVMap = {
  { "InitialHealth", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.initialHealth = opt.value_or(ab.initialHealth);
      return opt.has_value();
    }
  },
  { "MaxHealth", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.maxHealth = opt.value_or(ab.maxHealth);
      return opt.has_value();
    }
  },
  { "SubdualDamageCap", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.subdualDamageCap = opt.value_or(ab.subdualDamageCap);
      return opt.has_value();
    }
  },
  { "SubdualDamageHealAmount", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.subdualDamageHealAmount = opt.value_or(ab.subdualDamageHealAmount);
      return opt.has_value();
    }
  },
  { "SubdualDamageHealRate", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.subdualDamageHealRateMs = opt.value_or(ab.subdualDamageHealRateMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::Turret> AITurretKVMap = {
  { "AllowsPitch", [](Objects::Turret& t, INIFile& f) { t.canPitch = f.parseBool(); return true; } },
  { "ControlledWeaponSlots", [](Objects::Turret& t, INIFile& f) { return f.parseEnumSet<Objects::WeaponSlot>(t.controlledSlots, CALL(Objects::getWeaponSlot)); } },
  { "FirePitch", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.firePitch = opt.value_or(t.firePitch);
      return opt.has_value();
    }
  },
  { "FiresWhileTurning", [](Objects::Turret& t, INIFile& f) { t.canFireAndTurn = f.parseBool(); return true; } },
  { "GroundUnitPitch", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.groundUnitPitch = opt.value_or(t.groundUnitPitch);
      return opt.has_value();
    }
  },
  { "InitiallyDisabled", [](Objects::Turret& t, INIFile& f) { t.disabled = f.parseBool(); return true; } },
  { "MaxIdleScanAngle", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.maxScanAngle = opt.value_or(t.maxScanAngle);
      return opt.has_value();
    }
  },
  { "MaxIdleScanInterval", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseInteger();
      t.minScanIntervalMs = opt.value_or(t.minScanIntervalMs);
      return opt.has_value();
    }
  },
  { "MinIdleScanAngle", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.minScanAngle = opt.value_or(t.minScanAngle);
      return opt.has_value();
    }
  },
  { "MinIdleScanInterval", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseInteger();
      t.minScanIntervalMs = opt.value_or(t.minScanIntervalMs);
      return opt.has_value();
    }
  },
  { "MinPhysicalPitch", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.minPitch = opt.value_or(t.minPitch);
      return opt.has_value();
    }
  },
  { "NaturalTurretAngle", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.defaultAngle = opt.value_or(t.defaultAngle);
      return opt.has_value();
    }
  },
  { "NaturalTurretPitch", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.defaultPitch = opt.value_or(t.defaultPitch);
      return opt.has_value();
    }
  },
  { "RecenterTime", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseInteger();
      t.recenterTimeMs = opt.value_or(t.recenterTimeMs);
      return opt.has_value();
    }
  },
  { "TurretFireAngleSweep", [](Objects::Turret& t, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() != 2) {
        return false;
      }

      auto slot = Objects::getWeaponSlot(values[0]);
      auto opt = f.parseFloat(values[1]);
      if (!opt || !slot) {
        return false;
      }

      auto idx = weaponSlotToIdx(*slot);
      t.fireAngleSweep[idx] = *opt;

      return true;
    }
  },
  { "TurretSweepSpeedModifier", [](Objects::Turret& t, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() != 2) {
        return false;
      }

      auto slot = Objects::getWeaponSlot(values[0]);
      auto opt = f.parseFloat(values[1]);
      if (!opt || !slot) {
        return false;
      }

      auto idx = weaponSlotToIdx(*slot);
      t.sweepSpeedModifier[idx] = *opt;

      return true;
    }
  },
  { "TurretPitchRate", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.rotationRate.y = opt.value_or(0.0f);
      return opt.has_value();
    }
  },
  { "TurretTurnRate", [](Objects::Turret& t, INIFile& f) {
      auto opt = f.parseFloat();
      t.rotationRate.x = opt.value_or(0.0f);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::AI> AIKVMap = {
  { "AutoAcquireEnemiesWhenIdle", [](Objects::AI& aid, INIFile& f) {
      return f.parseEnumSet<Objects::AutoAcquireEnemyMode>(aid.acquireEnemiesWhenIdle, CALL(Objects::getAutoAcquireEnemyMode));
    }
  },
  { "ForbidPlayerCommands", [](Objects::AI& aid, INIFile& f) { aid.ignorePlayer = f.parseBool(); return true; } },
  { "MoodAttackCheckRate", [](Objects::AI& aid, INIFile& f) {
      auto opt = f.parseInteger();
      aid.moodAttackCheckRateMs = opt.value_or(aid.moodAttackCheckRateMs);
      return opt.has_value();
    }
  },
  { "Turret", [](Objects::AI& aid, INIFile& f) { return f.parseAttributeBlock(aid.turret1, AITurretKVMap); } }
};

static INIApplierMap<Objects::ArmorSet> ArmorSetKVMap = {
  { "Armor", [](Objects::ArmorSet& as, INIFile& f) { as.armor = f.parseString(); return !as.armor.empty(); } },
  { "Conditions", [](Objects::ArmorSet& as, INIFile& f) { return f.parseEnumSet<Objects::ArmorSet::Condition>(as.conditions, CALL(Objects::getArmorSetCondition)); } },
  { "DamageFX", [](Objects::ArmorSet& as, INIFile& f) { as.damage = f.parseString(); return !as.damage.empty(); } }
};

static INIApplierMap<Objects::AssistedTargeting> AssistedTargetingKVMap = {
  { "AssistingClipSize", [](Objects::AssistedTargeting& as, INIFile& f) {
      auto opt = f.parseSignedInteger();
      as.numShots = opt.value_or(as.numShots);
      return opt.has_value();
    }
  },
  { "AssistingWeaponSlot", [](Objects::AssistedTargeting& as, INIFile& f) {
      auto opt = f.parseEnum<Objects::WeaponSlot>(CALL(Objects::getWeaponSlot));
      as.slot = opt.value_or(as.slot);
      return opt.has_value();
    }
  },
  { "LaserFromAssisted", [](Objects::AssistedTargeting& as, INIFile& f) { as.laserFrom = f.parseString(); return !as.laserFrom.empty(); } },
  { "LaserToTarget", [](Objects::AssistedTargeting& as, INIFile& f) { as.laserTo = f.parseString(); return !as.laserTo.empty(); } },
};

static INIApplierMap<Objects::AssaultTransport> AssaultTransportKVMap = {
  { "MembersGetHealedAtLifeRatio", [](Objects::AssaultTransport& at, INIFile& f) {
      auto opt = f.parseFloat();
      at.healedWhenBelow = opt.value_or(at.healedWhenBelow);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::AutoDeposit> AutoDepositKVMap = {
  { "ActualMoney", [](Objects::AutoDeposit& ad, INIFile& f) { ad.actualMoney = f.parseBool(); return true; } },
  { "DepositAmount", [](Objects::AutoDeposit& ad, INIFile& f) {
      auto opt = f.parseSignedInteger();
      ad.amount = opt.value_or(ad.amount);
      return opt.has_value();
    }
  },
  { "DepositTiming", [](Objects::AutoDeposit& ad, INIFile& f) {
      auto opt = f.parseInteger();
      ad.intervalMs = opt.value_or(ad.intervalMs);
      return opt.has_value();
    }
  },
  { "InitialCaptureBonus", [](Objects::AutoDeposit& ad, INIFile& f) {
      auto opt = f.parseSignedShort();
      ad.captureBonus = opt.value_or(ad.captureBonus);
      return opt.has_value();
    }
  },
  { "UpgradedBoost", [](Objects::AutoDeposit& ad, INIFile& f) {
      auto values = f.parseAttributes();
      if (values.size() < 2) {
        return false;
      }

      if (!values.contains("UpgradeType") || !values.contains("Boost")) {
        return false;
      }

      auto opt = f.parseSignedInteger(values["Boost"]);
      if (!opt) {
        return false;
      }

      ad.boostValue = *opt;
      ad.upgrade = std::move(values["UpgradeType"]);

      return true;
    }
  },
};

static INIApplierMap<Objects::AutoFindHealing> AutoFindHealingKVMap = {
  { "AlwaysHeal", [](Objects::AutoFindHealing& ah, INIFile& f) {
      auto opt = f.parseFloat();
      ah.alwaysHeal = opt.value_or(ah.alwaysHeal);
      return opt.has_value();
    }
  },
  { "NeverHeal", [](Objects::AutoFindHealing& ah, INIFile& f) {
      auto opt = f.parseFloat();
      ah.neverHeal = opt.value_or(ah.neverHeal);
      return opt.has_value();
    }
  },
  { "ScanRange", [](Objects::AutoFindHealing& ah, INIFile& f) {
      auto opt = f.parseFloat();
      ah.scanRange = opt.value_or(ah.scanRange);
      return opt.has_value();
    }
  },
  { "ScanRate", [](Objects::AutoFindHealing& ah, INIFile& f) {
      auto opt = f.parseInteger();
      ah.scanIntervalMs = opt.value_or(ah.scanIntervalMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::AutoHeal> AutoHealKVMap = {
  { "AffectsWholePlayer", [](Objects::AutoHeal& ah, INIFile& f) { ah.wholePlayer = f.parseBool(); return true; } },
  { "ForbiddenKindOf", [](Objects::AutoHeal& ah, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(ah.healingExclusion, CALL(Objects::getAttribute));
    }
  },
  { "HealingAmount", [](Objects::AutoHeal& ah, INIFile& f) {
      auto opt = f.parseSignedInteger();
      ah.healingAmount = opt.value_or(ah.healingAmount);
      return opt.has_value();
    }
  },
  { "HealingDelay", [](Objects::AutoHeal& ah, INIFile& f) {
      auto opt = f.parseInteger();
      ah.healingDelayMs = opt.value_or(ah.healingDelayMs);
      return opt.has_value();
    }
  },
  { "KindOf", [](Objects::AutoHeal& ah, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(ah.healingInclusion, CALL(Objects::getAttribute));
    }
  },
  { "Radius", [](Objects::AutoHeal& ah, INIFile& f) {
      auto opt = f.parseFloat();
      ah.radius = opt.value_or(ah.radius);
      return opt.has_value();
    }
  },
  { "SkipSelfForHealing", [](Objects::AutoHeal& ah, INIFile& f) { ah.skipSelf = f.parseBool(); return true; } },
  { "StartsActive", [](Objects::AutoHeal& ah, INIFile& f) { ah.enabled = f.parseBool(); return true; } },
  { "*", [](Objects::AutoHeal&, INIFile& f) { f.parseString(); return true; } }
};

static INIApplierMap<Objects::BaikonurLaunchPower> BaikonurLaunchPowerKVMap = {
  { "DetonationObject", [](Objects::BaikonurLaunchPower& blp, INIFile& f) { blp.detonationObject = f.parseString(); return !blp.detonationObject.empty(); } }
};

static bool parseBoneFXItem(INIFile& f, Objects::BoneFXItems& items, size_t damageIdx, size_t itemIdx) {
  auto values = f.parseStringList();
  if (values.size() < 5) {
    return false;
  }

  if (!values[0].starts_with("bone:")) {
    return false;
  }

  Objects::BoneFXItem item;
  item.bone = values[0].substr(5);

  if (!values[1].starts_with("OnlyOnce:")) {
    return false;
  }
  item.once = f.parseBool(values[1].substr(9));

  auto opt = f.parseInteger(values[2]);
  if (!opt) {
    return false;
  }
  item.minDelayMs = *opt;

  opt = f.parseInteger(values[3]);
  if (!opt) {
    return false;
  }
  item.maxDelayMs = *opt;

  if (values[4].starts_with("FXList:")) {
    item.itemName = values[4].substr(7);
  } else if (values[4].starts_with("PSys:")) {
    item.itemName = values[4].substr(5);
  } else {
    return false;
  }

  items[damageIdx][itemIdx] = std::move(item);

  return true;
}

static INIApplierMap<Objects::BoneFX> BoneFXKVMap = {
  { "DamageFXTypes", [](Objects::BoneFX& bfx, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(bfx.damageEffectTypes, CALL(Objects::getDamageType));
    }
  },
  { "DamageParticleTypes", [](Objects::BoneFX& bfx, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(bfx.damageParticleTypes, CALL(Objects::getDamageType));
    }
  },
  { "RubbleFXList1", [](Objects::BoneFX& bfx, INIFile& f) { return parseBoneFXItem(f, bfx.effects, 3, 0); } },
  { "RubbleParticleSystem1", [](Objects::BoneFX& bfx, INIFile& f) { return parseBoneFXItem(f, bfx.particles, 3, 0); } },
};

static bool parseBridgeDieEffect(INIFile& f, std::list<Objects::BridgeDieItem>& list) {
  auto values = f.parseAttributes();
  if (values.size() < 3) {
    return false;
  }

  if (!values.contains("Delay") || !values.contains("Bone")) {
    return false;
  }

  Objects::BridgeDieItem item;
  item.bone = std::move(values["Bone"]);

  auto opt = f.parseInteger(values["Delay"]);
  if (!opt) {
    return false;
  }
  item.delayMs = *opt;

  if (values.contains("FX")) {
    item.name = std::move(values["FX"]);
  } else if (values.contains("OCL")) {
    item.name = std::move(values["OCL"]);
  } else {
    return false;
  }

  list.emplace_back(std::move(item));

  return true;
};

static INIApplierMap<Objects::Bridge> BridgeKVMap = {
  { "BridgeDieFX", [](Objects::Bridge& b, INIFile& f) {
      return parseBridgeDieEffect(f, b.dieEffects);
    }
  },
  { "BridgeDieOCL", [](Objects::Bridge& b, INIFile& f) {
      return parseBridgeDieEffect(f, b.dieCreationLists);
    }
  },
  { "LateralScaffoldSpeed", [](Objects::Bridge& b, INIFile& f) {
      auto opt = f.parseFloat();
      b.lateralScaffoldSpeed = opt.value_or(b.lateralScaffoldSpeed);
      return opt.has_value();
    }
  },
  { "VerticalScaffoldSpeed", [](Objects::Bridge& b, INIFile& f) {
      auto opt = f.parseFloat();
      b.verticalScaffoldSpeed = opt.value_or(b.verticalScaffoldSpeed);
      return opt.has_value();
    }
  },
};

// TODO
static INIApplierMap<Objects::BattlePlan> BattlePlanKVMap = {
  { "*", SKIP(Objects::BattlePlan) }
};

static INIApplierMap<Objects::CashBounty> CashBountyKVMap = {
  { "Bounty", [](Objects::CashBounty& cb, INIFile& f) {
      auto opt = f.parsePercent();
      cb.bounty = opt.value_or(cb.bounty);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::CashHack> CashHackKVMap = {
  { "MoneyAmount", [](Objects::CashHack& ch, INIFile& f) {
      auto opt = f.parseSignedInteger();
      ch.amount = opt.value_or(ch.amount);
      return opt.has_value();
    }
  },
  { "UpgradeMoneyAmount", [](Objects::CashHack& ch, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      auto valueOpt = f.parseSignedInteger(values[1]);
      if (!valueOpt) {
        return false;
      }

      ch.upgrades.emplace_back(std::make_pair(std::move(values[0]), *valueOpt));

      return true;
    }
  }
};

static INIApplierMap<Objects::ChinookAI> ChinookAIKVMap = {
  { "*", SKIP(Objects::ChinookAI) }
};

static INIApplierMap<Objects::CleanupArea> CleanupAreaKVMap = {
  { "*", SKIP(Objects::CleanupArea) }
};

static INIApplierMap<Objects::CleanupHazard> CleanupHazardKVMap = {
  { "*", SKIP(Objects::CleanupHazard) }
};

static std::optional<Objects::WeaponFX> parseWeaponEffect(INIFile& f) {
  auto value = f.parseStringList();
  if (value.size() != 2) {
    return {};
  }

  auto opt = Objects::getWeaponSlot(value[0]);
  if (!opt) {
    return {};
  }

  Objects::WeaponFX wfx;
  wfx.slot = *opt;
  wfx.effect = value[1];

  return {std::move(wfx)};
}

static std::optional<Objects::Animation> parseAnimation(INIFile& f) {
  auto values = f.parseStringList();
  if (values.empty()) {
    return {};
  }

  Objects::Animation anim;
  anim.name = std::move(values[0]);

  if (values.size() > 1) {
    auto opt = f.parseInteger(values[1]);
    if (!opt) {
      return {};
    }
    anim.distanceCovered = *opt;
  }
  if (values.size() > 2) {
    auto opt = f.parseInteger(values[2]);
    if (!opt) {
      return {};
    }
    anim.interval = *opt;
  }

  return {std::move(anim)};
};

static INIApplierMap<Objects::ConditionState> ConditionStateKVMap = {
  { "Animation", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseAnimation(f);
      if (!opt) {
        return false;
      }

      cs.animation = std::make_shared<Objects::Animation>(std::move(*opt));

      return true;
    }
  },
  { "AnimationMode", [](Objects::ConditionState& cs, INIFile& f) {
      auto modeString = f.parseString();
      auto opt = Objects::getAnimationMode(modeString);
      cs.animationMode = opt.value_or(Objects::AnimationMode::NONE);

      return opt.has_value();
    }
  },
  { "AnimationSpeedFactorRange", [](Objects::ConditionState& cs, INIFile& f) {
      auto range = f.parseStringList();
      if (range.size() != 2) {
        return false;
      }

      auto opt = f.parseFloat(range[0]);
      if (!opt) {
        return false;
      }
      cs.minAnimationSpeed = *opt;

      opt = f.parseFloat(range[1]);
      if (!opt) {
        return false;
      }
      cs.maxAnimationSpeed = *opt;

      return true;
    }
  },
  { "Flags", [](Objects::ConditionState& cs, INIFile& f) {
      auto flagStrings = f.parseStringList();
      if (flagStrings.empty()) {
        return false;
      }

      std::set<Objects::AnimationFrameMode> modes;
      for (auto& flagString : flagStrings) {
        auto mode = Objects::getAnimationFrameMode(flagString);
        if (!mode) {
          return false;
        }
        modes.insert(*mode);
      }

      cs.flags = std::move(modes);

      return true;
    }
  },
  { "HideSubObject", [](Objects::ConditionState& cs, INIFile& f) {
      cs.hiddenSubObjects = f.parseStringList();
      return !cs.hiddenSubObjects.empty();
    }
  },
  { "IdleAnimation", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseAnimation(f);
      if (!opt) {
        return false;
      }

      cs.idleAnimations.emplace_back(std::move(*opt));

      return true;
    }
  },
  { "Model", [](Objects::ConditionState& cs, INIFile& f) {
      // Garbage around
      cs.model = f.parseLooseValue();
      if (cs.model == "NONE" || cs.model == "None") {
        cs.model = "";
        return true;
      }

      return !cs.model.empty();
    }
  },
  { "ParticleSysBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto value = f.parseStringList();
      if (value.size() != 2) {
        return false;
      }

      cs.particleBones.emplace_back(std::move(value));
      return true;
    }
  },
  { "ShowSubObject", [](Objects::ConditionState& cs, INIFile& f) {
      cs.shownSubObjects = f.parseStringList();
      return !cs.shownSubObjects.empty();
    }
  },
  { "TransitionKey", [](Objects::ConditionState& cs, INIFile& f) { cs.transitionKey = f.parseString(); return !cs.transitionKey.empty(); } },
  { "Turret", [](Objects::ConditionState& cs, INIFile& f) { cs.turret1.name = f.parseString(); return !cs.turret1.name.empty(); } },
  { "TurretArtAngle", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = f.parseFloat();
      cs.turret1.artAngle = opt.value_or(cs.turret1.artAngle);
      return opt.has_value();
    }
  },
  { "TurretPitch", [](Objects::ConditionState& cs, INIFile& f) {
      cs.turret1.pitchName = f.parseString();
      return !cs.turret1.pitchName.empty();
    }
  },
  { "WaitForStateToFinishIfPossible", [](Objects::ConditionState& cs, INIFile& f) {
      cs.waitForState = f.parseString();
      return !cs.waitForState.empty();
    }
  },
  { "WeaponFireFXBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseWeaponEffect(f);
      if (opt) {
        cs.weaponFireEffectBones.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "WeaponHideShowBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseWeaponEffect(f);
      if (opt) {
        cs.weaponHideShowBones.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "WeaponLaunchBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseWeaponEffect(f);
      if (opt) {
        cs.weaponLaunchBones.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "WeaponMuzzleFlash", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseWeaponEffect(f);
      if (opt) {
        cs.weaponMuzzleFlashBones.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "WeaponRecoilBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto opt = parseWeaponEffect(f);
      if (opt) {
        cs.weaponRecoilBones.push_back(*opt);
      }
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::CommandSetUpgrade> CommandSetUpgradeKVMap = {
  { "CommandSet", [](Objects::CommandSetUpgrade& csu, INIFile& f) { csu.commandSet1 = f.parseString(); return !csu.commandSet1.empty(); } },
  { "CommandSetAlt", [](Objects::CommandSetUpgrade& csu, INIFile& f) { csu.commandSet2 = f.parseString(); return !csu.commandSet2.empty(); } },
  { "TriggerAlt", [](Objects::CommandSetUpgrade& csu, INIFile& f) { csu.altTrigger = f.parseString(); return !csu.altTrigger.empty(); } }
};

static INIApplierMap<Objects::CostModifierUpgrade> CostModifierUpgradeKVMap = {
  { "EffectKindOf", [](Objects::CostModifierUpgrade& cmu, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(cmu.affecting, CALL(Objects::getAttribute));
    }
  },
  { "Percentage", [](Objects::CostModifierUpgrade& cmu, INIFile& f) {
      auto opt = f.parsePercent();
      cmu.percentage = opt.value_or(cmu.percentage);
      return opt.has_value();
    }
  },
};

// TODO
static INIApplierMap<Objects::Countermeasure> CountermeasureKVMap = {
  { "*", SKIP(Objects::Countermeasure) }
};

static INIApplierMap<Objects::CrateCollision> CrateCollisionKVMap = {
  { "ForbiddenKindOf", [](Objects::CrateCollision& cc, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(cc.collisionExclusion, CALL(Objects::getAttribute));
    }
  },
  { "RequiredKindOf", [](Objects::CrateCollision& cc, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(cc.collisionInclusion, CALL(Objects::getAttribute));
    }
  }
};

static INIApplierMap<Objects::CreateCrateDie> CreateCrateDieKVMap = {
  { "CrateData", [](Objects::CreateCrateDie& cd, INIFile& f) {
      cd.crate = f.parseString();
      return !cd.crate.empty();
    }
  }
};

static INIApplierMap<Objects::CreateObjectDie> CreateObjectDieKVMap = {
  { "CreationList", [](Objects::CreateObjectDie& cod, INIFile& f) {
      cod.creationList = f.parseString();
      return !cod.creationList.empty();
    }
  },
  { "TransferPreviousHealth", [](Objects::CreateObjectDie& cod, INIFile& f) { cod.transferHealth = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::CrushDie> CrushDieKVMap = {
  { "BackEndCrushSound", [](Objects::CrushDie& cd, INIFile& f) { cd.backEndCrushSound = f.parseString(); return !cd.backEndCrushSound.empty(); } },
  { "BackEndCrushSoundPercent", [](Objects::CrushDie& cd, INIFile& f) {
      auto opt = f.parsePercent();
      cd.totalBackEndSound = opt.value_or(cd.totalBackEndSound);
      return opt.has_value();
    }
  },
  { "FrontEndCrushSound", [](Objects::CrushDie& cd, INIFile& f) { cd.frontEndCrushSound = f.parseString(); return !cd.frontEndCrushSound.empty(); } },
  { "FrontEndCrushSoundPercent", [](Objects::CrushDie& cd, INIFile& f) {
      auto opt = f.parsePercent();
      cd.totalFrontEndSound = opt.value_or(cd.totalFrontEndSound);
      return opt.has_value();
    }
  },
  { "TotalCrushSound", [](Objects::CrushDie& cd, INIFile& f) { cd.crushSound = f.parseString(); return !cd.crushSound.empty(); } },
  { "TotalCrushSoundPercent", [](Objects::CrushDie& cd, INIFile& f) {
      auto opt = f.parsePercent();
      cd.totalCrushSound = opt.value_or(cd.totalCrushSound);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::DefaultProductionExit> DefaultProductionExitKVMap = {
  { "NaturalRallyPoint", [](Objects::DefaultProductionExit& dp, INIFile& f) {
      auto coords = f.parseCoord3D();
      dp.rallyPoint.x = coords[0];
      dp.rallyPoint.y = coords[2];
      dp.rallyPoint.z = coords[1];

      return true;
    }
  },
  { "UnitCreatePoint", [](Objects::DefaultProductionExit& dp, INIFile& f) {
      auto coords = f.parseCoord3D();
      dp.creationPoint.x = coords[0];
      dp.creationPoint.y = coords[2];
      dp.creationPoint.z = coords[1];

      return true;
    }
  },
  { "UseSpawnRallyPoint", [](Objects::DefaultProductionExit& dp, INIFile& f) { dp.useRallyPoint = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::Deletion> DeletionKVMap = {
  { "MaxLifetime", [](Objects::Deletion& d, INIFile& f) {
      auto opt = f.parseInteger();
      d.maxLifetimeMs = opt.value_or(d.maxLifetimeMs);
      return opt.has_value();
    }
  },
  { "MinLifetime", [](Objects::Deletion& d, INIFile& f) {
      auto opt = f.parseInteger();
      d.minLifetimeMs = opt.value_or(d.minLifetimeMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::DeliverPayload> DeliverPayloadKVMap = {
  { "DeliveryDistance", [](Objects::DeliverPayload& dp, INIFile& f) {
      auto opt = f.parseFloat();
      dp.distance = opt.value_or(dp.distance);
      return opt.has_value();
    }
  },
  { "DoorDelay", [](Objects::DeliverPayload& dp, INIFile& f) {
      auto opt = f.parseInteger();
      dp.doorDelayMs = opt.value_or(dp.doorDelayMs);
      return opt.has_value();
    }
  },
  { "DropDelay", [](Objects::DeliverPayload& dp, INIFile& f) {
      auto opt = f.parseInteger();
      dp.dropDelayMs = opt.value_or(dp.dropDelayMs);
      return opt.has_value();
    }
  },
  { "DropOffset", [](Objects::DeliverPayload& dp, INIFile& f) {
      auto coords = f.parseCoord3D();
      dp.offset.x = coords[0];
      dp.offset.y = coords[2];
      dp.offset.z = coords[1];

      return true;
    }
  },
  { "MaxAttempts", [](Objects::DeliverPayload& dp, INIFile& f) {
      auto opt = f.parseSignedInteger();
      dp.attempts = opt.value_or(dp.attempts);
      return opt.has_value();
    }
  },
  { "PutInContainer", [](Objects::DeliverPayload& dp, INIFile& f) { dp.dropCarrier = f.parseString(); return !dp.dropCarrier.empty(); } },
};

static INIApplierMap<Objects::DemoTrap> DemoTrapKVMap = {
  { "AutoDetonationWithFriendsInvolved", [](Objects::DemoTrap& dt, INIFile& f) { dt.detonateWithAllies = f.parseBool(); return true; } },
  { "DefaultProximityMode", [](Objects::DemoTrap& dt, INIFile& f) { dt.defaultProximity = f.parseBool(); return true; } },
  { "DetonateWhenKilled", [](Objects::DemoTrap& dt, INIFile& f) { dt.detonateOnDeath = f.parseBool(); return true; } },
  { "DetonationWeapon", [](Objects::DemoTrap& dt, INIFile& f) { dt.detonationWeapon = f.parseString(); return !dt.detonationWeapon.empty(); } },
  { "DetonationWeaponSlot", [](Objects::DemoTrap& dt, INIFile& f) {
      auto opt = f.parseEnum<Objects::WeaponSlot>(CALL(Objects::getWeaponSlot));
      dt.detonation = opt.value_or(Objects::WeaponSlot::NONE);
      return opt.has_value();
    }
  },
  { "IgnoreTargetTypes", [](Objects::DemoTrap& dt, INIFile& f) { return f.parseEnumSet<Objects::Attribute>(dt.triggerExclusions, CALL(Objects::getAttribute)); } },
  { "ManualModeWeaponSlot", [](Objects::DemoTrap& dt, INIFile& f) {
      auto opt = f.parseEnum<Objects::WeaponSlot>(CALL(Objects::getWeaponSlot));
      dt.manual = opt.value_or(Objects::WeaponSlot::NONE);
      return opt.has_value();
    }
  },
  { "ProximityModeWeaponSlot", [](Objects::DemoTrap& dt, INIFile& f) {
      auto opt = f.parseEnum<Objects::WeaponSlot>(CALL(Objects::getWeaponSlot));
      dt.proximity = opt.value_or(Objects::WeaponSlot::NONE);
      return opt.has_value();
    }
  },
  { "TriggerDetonationRange", [](Objects::DemoTrap& dt, INIFile& f) {
      auto opt = f.parseFloat();
      dt.triggerRange = opt.value_or(dt.triggerRange);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::Die> DieKVMap = {
  { "DeathTypes", [](Objects::Die& d, INIFile& f) {
      return f.parseEnumSet<Objects::DeathType>(d.deathTypes, CALL(Objects::getDeathType));
    }
  },
  { "ExemptStatus", [](Objects::Die& d, INIFile& f) {
      return f.parseEnumSet<Objects::Status>(d.excludedStates, CALL(Objects::getStatus));
    }
  },
  { "RequiredStatus", [](Objects::Die& d, INIFile& f) {
      return f.parseEnumSet<Objects::Status>(d.requiredStates, CALL(Objects::getStatus));
    }
  },
  { "VeterancyLevels", [](Objects::Die& d, INIFile& f) {
      return f.parseEnumSet<Objects::Veterancy>(d.veterancyLevels, CALL(Objects::getVeterancy));
    }
  }
};

static INIApplierMap<Objects::DeployStyleAI> DeployStyleAIKVMap = {
  { "PackTime", [](Objects::DeployStyleAI& ds, INIFile& f) {
      auto value = f.parseInteger();
      ds.packTimeMs = value.value_or(ds.packTimeMs);
      return value.has_value();
    }
  },
  { "ManualDeployAnimations", [](Objects::DeployStyleAI& ds, INIFile& f) { ds.manualDeployAnimation = f.parseBool(); return true; } },
  { "TurretsFunctionOnlyWhenDeployed", [](Objects::DeployStyleAI& ds, INIFile& f) { ds.deployRequired = f.parseBool(); return true; } },
  { "TurretsMustCenterBeforePacking", [](Objects::DeployStyleAI& ds, INIFile& f) { ds.centerBeforePacking = f.parseBool(); return true; } },
  { "UnpackTime", [](Objects::DeployStyleAI& ds, INIFile& f) {
      auto value = f.parseInteger();
      ds.unpackTimeMs = value.value_or(ds.unpackTimeMs);
      return value.has_value();
    }
  },
};

static INIApplierMap<Objects::DestroyDie> DestroyDieKVMap = {
};

static INIApplierMap<Objects::Dock> DockKVMap = {
  { "AllowsPassthrough", [](Objects::Dock& d, INIFile& f) { d.allowPassThrough = f.parseBool(); return true; } },
  { "NumberApproachPositions", [](Objects::Dock& d, INIFile& f) {
      auto value = f.parseSignedInteger();
      d.numApproachPositions = value.value_or(d.numApproachPositions);
      return value.has_value();
    }
  }
};

static INIApplierMap<Objects::DozerAI> DozerAIKVMap = {
  { "BoredRange", [](Objects::DozerAI& dz, INIFile& f) {
      auto opt = f.parseFloat();
      dz.boredRange = opt.value_or(dz.boredRange);
      return opt.has_value();
    }
  },
  { "BoredTime", [](Objects::DozerAI& dz, INIFile& f) {
      auto opt = f.parseFloat();
      dz.boredTime = opt.value_or(dz.boredTime);
      return opt.has_value();
    }
  },
  { "RepairHealthPercentPerSecond", [](Objects::DozerAI& dz, INIFile& f) {
      auto opt = f.parseFloat();
      dz.repairPerSecond = opt.value_or(dz.repairPerSecond);
      return opt.has_value();
    }
  }
};

// TODO
static INIApplierMap<Objects::DynamicShroudClearingRange> DynamicShroudClearingRangeKVMap = {
  { "GridDecalTemplate", [](Objects::DynamicShroudClearingRange& dr, INIFile& f) {
      return f.parseAttributeBlock(dr, DynamicShroudClearingRangeKVMap);
    }
  },
  { "*", SKIP(Objects::DynamicShroudClearingRange) }
};

static INIApplierMap<Objects::EjectPilotDie> EjectPilotDieKVMap = {
  { "AirCreationList", [](Objects::EjectPilotDie& ed, INIFile& f) {
      ed.airCreationList = f.parseString();
      return !ed.airCreationList.empty();
    }
  },
  { "GroundCreationList", [](Objects::EjectPilotDie& ed, INIFile& f) {
      ed.groundCreationList = f.parseString();
      return !ed.groundCreationList.empty();
    }
  }
};

static INIApplierMap<Objects::ExperienceScalarUpgrade> ExperienceScalarUpgradeKVMap = {
  { "AddXPScalar", [](Objects::ExperienceScalarUpgrade& xu, INIFile& f) {
      auto opt = f.parseFloat();
      xu.xpScalar = opt.value_or(xu.xpScalar);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::FireWeaponCollision> FireWeaponCollisionDataKVMap = {
  { "CollideWeapon", [](Objects::FireWeaponCollision& c, INIFile& f) { c.weapon = f.parseString(); return !c.weapon.empty(); } },
  { "RequiredStatus", [](Objects::FireWeaponCollision& c, INIFile& f) {
      return f.parseEnumSet<Objects::Status>(c.requiredStates, CALL(Objects::getStatus));
    }
  },
};

static INIApplierMap<Objects::FireWeaponWhenDamaged> FireWeaponWhenDamagedKVMap = {
  { "ContinuousWeaponPristine", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.continuousWeaponPristine = f.parseString(); return !fwd.continuousWeaponPristine.empty(); } },
  { "ContinuousWeaponDamaged", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.continuousWeaponDamaged = f.parseString(); return !fwd.continuousWeaponDamaged.empty(); } },
  { "ContinuousWeaponReallyDamaged", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.continuousWeaponReallyDamaged = f.parseString(); return !fwd.continuousWeaponReallyDamaged.empty(); } },
  { "ContinuousWeaponRubble", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.continuousWeaponRubble = f.parseString(); return !fwd.continuousWeaponRubble.empty(); } },
  { "DamageAmount", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) {
      auto opt = f.parseFloat();
      fwd.damageAmount = opt.value_or(fwd.damageAmount);
      return opt.has_value();
    }
  },
  { "DamageTypes", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(fwd.damageTypes, CALL(Objects::getDamageType));
    }
  },
  { "ReactionWeaponPristine", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.reactionWeaponPristine = f.parseString(); return !fwd.reactionWeaponPristine.empty(); } },
  { "ReactionWeaponDamaged", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.reactionWeaponDamaged = f.parseString(); return !fwd.reactionWeaponDamaged.empty(); } },
  { "ReactionWeaponReallyDamaged", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.reactionWeaponReallyDamaged = f.parseString(); return !fwd.reactionWeaponReallyDamaged.empty(); } },
  { "ReactionWeaponRubble", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.reactionWeaponRubble = f.parseString(); return !fwd.reactionWeaponRubble.empty(); } },
  { "StartsActive", [](Objects::FireWeaponWhenDamaged& fwd, INIFile& f) { fwd.active = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::FireWeaponWhenDead> FireWeaponWhenDeadKVMap = {
  { "DeathWeapon", [](Objects::FireWeaponWhenDead& fwd, INIFile& f) { fwd.weapon = f.parseString(); return !fwd.weapon.empty(); } },
  { "StartsActive", [](Objects::FireWeaponWhenDead& fwd, INIFile& f) { fwd.active = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::FireSpread> FireSpreadKVMap = {
  { "OCLEmbers", [](Objects::FireSpread& fs, INIFile& f) { fs.creationList = f.parseString(); return !fs.creationList.empty(); } },
  { "MaxSpreadDelay", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseInteger();
      fs.maxSpreadDelayMs = opt.value_or(fs.maxSpreadDelayMs);
      return opt.has_value();
    }
  },
  { "MinSpreadDelay", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseInteger();
      fs.minSpreadDelayMs = opt.value_or(fs.minSpreadDelayMs);
      return opt.has_value();
    }
  },
  { "SpreadTryRange", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseFloat();
      fs.spreadRange = opt.value_or(fs.spreadRange);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::FireWeapon> FireWeaponKVMap = {
  { "ExclusiveWeaponDelay", [](Objects::FireWeapon& fw, INIFile& f) {
      auto opt = f.parseInteger();
      fw.exclusiveWeaponDelayMs = opt.value_or(fw.exclusiveWeaponDelayMs);
      return opt.has_value();
    }
  },
  { "InitialDelay", [](Objects::FireWeapon& fw, INIFile& f) {
      auto opt = f.parseInteger();
      fw.initialDelayMs = opt.value_or(fw.initialDelayMs);
      return opt.has_value();
    }
  },
  { "Weapon", [](Objects::FireWeapon& fw, INIFile& f) { fw.weapon = f.parseString(); return !fw.weapon.empty(); } },
};

static INIApplierMap<Objects::Flammable> FlammableDataKVMap = {
  { "AflameDamageAmount", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseSignedInteger();
      fd.burningDamageAmount = opt.value_or(fd.burningDamageAmount);
      return opt.has_value();
    }
  },
  { "AflameDamageDelay", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.burningDamageDelayMs = opt.value_or(fd.burningDamageDelayMs);
      return opt.has_value();
    }
  },
  { "AflameDuration", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.burningDurationMs = opt.value_or(fd.burningDurationMs);
      return opt.has_value();
    }
  },
  { "BurnedDelay", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.burnedDelayMs = opt.value_or(fd.burnedDelayMs);
      return opt.has_value();
    }
  },
  { "BurningSoundName", [](Objects::Flammable& fd, INIFile& f) { fd.sound = f.parseString(); return !fd.sound.empty(); } },
  { "FlameDamageExpiration", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.damageExpirationMs = opt.value_or(fd.damageExpirationMs);
      return opt.has_value();
    }
  },
  { "FlameDamageLimit", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseFloat();
      fd.damageLimit = opt.value_or(fd.damageLimit);
      return opt.has_value();
    }
  }
};

// TODO
static INIApplierMap<Objects::FlightDeck> FlightDeckKVMap = {
  { "*", SKIP(Objects::FlightDeck) }
};

static INIApplierMap<Objects::Float> FloatKVMap = {
  { "Enabled", [](Objects::Float& fl, INIFile& f) { fl.enabled = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::FXListDie> FXListDieKVMap = {
  { "DeathFX", [](Objects::FXListDie& fld, INIFile& f) { fld.effect = f.parseString(); return !fld.effect.empty(); } },
  { "OrientToObject", [](Objects::FXListDie& fld, INIFile& f) { fld.orientToObject = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::GarrisonContain> GarrisonContainKVMap = {
  { "ImmuneToClearBuildingAttacks", [](Objects::GarrisonContain& gc, INIFile& f) { gc.noRaidAttack = f.parseBool(); return true; } },
  { "InitialRoster", [](Objects::GarrisonContain& gc, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      gc.initialRoster = values[0];

      auto opt = f.parseInteger(values[1]);
      gc.numInitial = opt.value_or(gc.numInitial);
      return opt.has_value();
    }
  },
  { "IsEnclosingContainer", [](Objects::GarrisonContain& gc, INIFile& f) { gc.enclosing = f.parseBool(); return true; } },
  { "MobileGarrison", [](Objects::GarrisonContain& gc, INIFile& f) { gc.mobile = f.parseBool(); return true; } }
};

// TODO
static INIApplierMap<Objects::GenerateMinefield> GenerateMinefieldKVMap = {
  { "*", SKIP(Objects::GenerateMinefield) }
};

static INIApplierMap<Objects::GrantScienceUpgrade> GrantScienceUpgradeKVMap = {
  { "GrantScience", [](Objects::GrantScienceUpgrade& su, INIFile& f) { su.science = f.parseString(); return !su.science.empty(); } }
};

static INIApplierMap<Objects::GrantUpgrade> GrantUpgradeKVMap = {
  { "ExemptStatus", [](Objects::GrantUpgrade& gu, INIFile& f) { return f.parseEnumSet<Objects::Status>(gu.exclusions, CALL(Objects::getStatus)); } },
  { "UpgradeToGrant", [](Objects::GrantUpgrade& gu, INIFile& f) { gu.upgrade = f.parseString(); return !gu.upgrade.empty(); } }
};

static INIApplierMap<Objects::HealContain> HealContainKVMap = {
  { "TimeForFullHeal", [](Objects::HealContain& hc, INIFile& f) {
      auto opt = f.parseFloat();
      hc.timeToFullHealthMs = opt.value_or(hc.timeToFullHealthMs);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::HeightDie> HeightDieKVMap = {
  { "DestroyAttachedParticlesAtHeight", [](Objects::HeightDie& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.destroyParticlesAt = opt.value_or(hd.destroyParticlesAt);
      return opt.has_value();
    }
  },
  { "TargetHeight", [](Objects::HeightDie& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.targetHeight = opt.value_or(hd.targetHeight);
      return opt.has_value();
    }
  },
  { "TargetHeightIncludesStructures", [](Objects::HeightDie& hd, INIFile& f) { hd.targetHeightForStructures = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::HelixContain> HelixContainKVMap = {
};

static INIApplierMap<Objects::HiveStructureBody> HiveStructureBodyKVMap = {
  { "PropagateDamageTypesToSlavesWhenExisting", [](Objects::HiveStructureBody& hs, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(hs.propagateDamages, CALL(Objects::getDamageType));
    }
  },
  { "SwallowDamageTypesIfSlavesNotExisting", [](Objects::HiveStructureBody& hs, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(hs.absorbDamages, CALL(Objects::getDamageType));
    }
  }
};

static INIApplierMap<Objects::HelicopterSlowDeath> HelicopterSlowDeathKVMap = {
  { "AttachParticle", [](Objects::HelicopterSlowDeath& hd, INIFile& f) { hd.particles = f.parseString(); return !hd.particles.empty(); } },
  { "AttachParticleBone", [](Objects::HelicopterSlowDeath& hd, INIFile& f) { hd.particlesBone = f.parseString(); return !hd.particlesBone.empty(); } },
  { "BladeBoneName", [](Objects::HelicopterSlowDeath& hd, INIFile& f) { hd.bladesBone = f.parseString(); return !hd.bladesBone.empty(); } },
  { "BladeObjectName", [](Objects::HelicopterSlowDeath& hd, INIFile& f) { hd.blades = f.parseString(); return !hd.blades.empty(); } },
  { "DelayFromGroundToFinalDeath", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.delayToBlowupMs = opt.value_or(hd.delayToBlowupMs);
      return opt.has_value();
    }
  },
  { "FallHowFast", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parsePercent();
      hd.fallSpeed = opt.value_or(hd.fallSpeed);
      return opt.has_value();
    }
  },
  { "FinalRubbleObject", [](Objects::HelicopterSlowDeath& hd, INIFile& f) { hd.rubble = f.parseString(); return !hd.rubble.empty(); } },
  { "FXBlade", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.bladeEffect = f.parseString();
      return !hd.bladeEffect.empty();
    }
  },
  { "FXFinalBlowUp", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.finalBlowupEffect = f.parseString();
      return !hd.finalBlowupEffect.empty();
    }
  },
  { "FXHitGround", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.hitGroundEffect = f.parseString();
      return !hd.hitGroundEffect.empty();
    }
  },
  { "MaxBladeFlyOffDelay", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.maxBladeFallOffDelayMs = opt.value_or(hd.maxBladeFallOffDelayMs);
      return opt.has_value();
    }
  },
  { "MaxBraking", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.maxBraking = opt.value_or(hd.maxBraking);
      return opt.has_value();
    }
  },
  { "MaxSelfSpin", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.maxSpin = opt.value_or(hd.maxSpin);
      return opt.has_value();
    }
  },
  { "MinBladeFlyOffDelay", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.minBladeFallOffDelayMs = opt.value_or(hd.minBladeFallOffDelayMs);
      return opt.has_value();
    }
  },
  { "MinSelfSpin", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.minSpin = opt.value_or(hd.minSpin);
      return opt.has_value();
    }
  },
  { "OCLBlade", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.bladeCreationList = f.parseString();
      return !hd.bladeCreationList.empty();
    }
  },
  { "OCLEjectPilot", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.ejectPilotCreationList = f.parseString();
      return !hd.ejectPilotCreationList.empty();
    }
  },
  { "OCLFinalBlowUp", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.finalBlowupCreationList = f.parseString();
      return !hd.finalBlowupCreationList.empty();
    }
  },
  { "OCLHitGround", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.hitGroundCreationList = f.parseString();
      return !hd.hitGroundCreationList.empty();
    }
  },
  { "SelfSpinUpdateAmount", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.spinUpdateAmount = opt.value_or(hd.spinUpdateAmount);
      return opt.has_value();
    }
  },
  { "SelfSpinUpdateDelay", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.spinUpdateDelayFrames = opt.value_or(hd.spinUpdateDelayFrames);
      return opt.has_value();
    }
  },
  { "SpiralOrbitForwardSpeed", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.spiralForwardSpeed = opt.value_or(hd.spiralForwardSpeed);
      return opt.has_value();
    }
  },
  { "SpiralOrbitForwardSpeedDamping", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.spiralForwardSpeedDampening = opt.value_or(hd.spiralForwardSpeedDampening);
      return opt.has_value();
    }
  },
  { "SpiralOrbitTurnRate", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.spiralTurnRate = opt.value_or(hd.spiralTurnRate);
      return opt.has_value();
    }
  },
  { "SoundDeathLoop", [](Objects::HelicopterSlowDeath& hd, INIFile& f) {
      hd.deathSound = f.parseString();
      return !hd.deathSound.empty();
    }
  }
};

static INIApplierMap<Objects::Horde> HordeKVMap = {
  { "Action", [](Objects::Horde& hd, INIFile& f) {
      hd.action = f.parseString();
      return !hd.action.empty();
    }
  },
  { "AlliesOnly", [](Objects::Horde& hd, INIFile& f) { hd.alliesOnly = f.parseBool(); return true; } },
  { "Count", [](Objects::Horde& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.minCount = opt.value_or(hd.minCount);
      return opt.has_value();
    }
  },
  { "ExactMatch", [](Objects::Horde& hd, INIFile& f) { hd.exactMatch = f.parseBool(); return true; } },
  { "KindOf", [](Objects::Horde& hd, INIFile& f) { return f.parseEnumSet<Objects::Attribute>(hd.kindOf, CALL(Objects::getAttribute)); } },
  { "Radius", [](Objects::Horde& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.radius = opt.value_or(hd.radius);
      return opt.has_value();
    }
  },
  { "RubOffRadius", [](Objects::Horde& hd, INIFile& f) {
      auto opt = f.parseFloat();
      hd.rubOffRadius = opt.value_or(hd.rubOffRadius);
      return opt.has_value();
    }
  },
  { "UpdateRate", [](Objects::Horde& hd, INIFile& f) {
      auto opt = f.parseInteger();
      hd.updateIntervalMs = opt.value_or(hd.updateIntervalMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::InstantDeath> InstantDeathKVMap = {
  { "FX", [](Objects::InstantDeath& id, INIFile& f) {
      auto value = f.parseString();
      if (value.empty()) {
        return false;
      }

      id.effects.emplace_back(std::move(value));
      return true;
    }
  },
  { "OCL", [](Objects::InstantDeath& id, INIFile& f) {
      auto value = f.parseString();
      if (value.empty()) {
        return false;
      }

      id.creationLists.emplace_back(std::move(value));
      return true;
    }
  }
};

// TODO
static INIApplierMap<Objects::JetAI> JetAIKVMap = {
  { "*", SKIP(Objects::JetAI) }
};

static INIApplierMap<Objects::JetSlowDeath> JetSlowDeathKV = {
  { "DelayFinalBlowUpFromHitGround", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parseInteger();
      jd.delayToBlowupMs = opt.value_or(jd.delayToBlowupMs);
      return opt.has_value();
    }
  },
  { "DelaySecondaryFromInitialDeath", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parseInteger();
      jd.delayToSecondaryDeathMs = opt.value_or(jd.delayToSecondaryDeathMs);
      return opt.has_value();
    }
  },
  { "FallHowFast", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parsePercent();
      jd.fallSpeed = opt.value_or(jd.fallSpeed);
      return opt.has_value();
    }
  },
  { "FXFinalBlowUp", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.finalBlowupEffect = f.parseString();
      return !jd.finalBlowupEffect.empty();
    }
  },
  { "FXHitGround", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.hitGroundEffect = f.parseString();
      return !jd.hitGroundEffect.empty();
    }
  },
  { "FXInitialDeath", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.initialDeathEffect = f.parseString();
      return !jd.initialDeathEffect.empty();
    }
  },
  { "FXOnGroundDeath", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.groundDeathEffect = f.parseString();
      return !jd.groundDeathEffect.empty();
    }
  },
  { "FXSecondary", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.secondaryEffect = f.parseString();
      return !jd.secondaryEffect.empty();
    }
  },
  { "OCLFinalBlowUp", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.finalBlowupCreationList = f.parseString();
      return !jd.finalBlowupCreationList.empty();
    }
  },
  { "OCLHitGround", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.hitGroundCreationList = f.parseString();
      return !jd.hitGroundCreationList.empty();
    }
  },
  { "OCLInitialDeath", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.initialDeathCreationList = f.parseString();
      return !jd.initialDeathCreationList.empty();
    }
  },
  { "OCLOnGroundDeath", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.groundDeathCreationList = f.parseString();
      return !jd.groundDeathCreationList.empty();
    }
  },
  { "OCLSecondary", [](Objects::JetSlowDeath& jd, INIFile& f) {
      jd.secondaryCreationList = f.parseString();
      return !jd.secondaryCreationList.empty();
    }
  },
  { "PitchRate", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parseFloat();
      jd.pitchRate = opt.value_or(jd.pitchRate);
      return opt.has_value();
    }
  },
  { "RollRate", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parseFloat();
      jd.rollRate = opt.value_or(jd.rollRate);
      return opt.has_value();
    }
  },
  { "RollRateDelta", [](Objects::JetSlowDeath& jd, INIFile& f) {
      auto opt = f.parsePercent();
      jd.rollRateDelta = opt.value_or(jd.rollRateDelta);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::Laser> LaserKVMap = {
  { "MuzzleParticleSystem", [](Objects::Laser& ld, INIFile& f) {
      ld.muzzleParticleSystem = f.parseString();
      return !ld.muzzleParticleSystem.empty();
    }
  },
  { "PunchThroughScalar", [](Objects::Laser& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.punchThroughScalar = opt.value_or(ld.punchThroughScalar);
      return opt.has_value();
    }
  },
  { "TargetParticleSystem", [](Objects::Laser& ld, INIFile& f) {
      ld.targetParticleSystem = f.parseString();
      return !ld.targetParticleSystem.empty();
    }
  }
};

static INIApplierMap<Objects::LaserDrawData> LaserDrawDataKVMap = {
  { "ArcHeight", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.arcHeight = opt.value_or(ld.arcHeight);
      return opt.has_value();
    }
  },
  { "InnerBeamWidth", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.innerBeamWidth = opt.value_or(ld.innerBeamWidth);
      return opt.has_value();
    }
  },
  { "InnerColor", [](Objects::LaserDrawData& ld, INIFile& f) { ld.innerColor = f.parseRGB(); return true; } },
  { "NumBeams", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseInteger();
      ld.numBeams = opt.value_or(ld.numBeams);
      return opt.has_value();
    }
  },
  { "OuterBeamWidth", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.outerBeamWidth = opt.value_or(ld.outerBeamWidth);
      return opt.has_value();
    }
  },
  { "OuterColor", [](Objects::LaserDrawData& ld, INIFile& f) { ld.outerColor = f.parseRGB(); return true; } },
  { "ScrollRate", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.scrollRate = opt.value_or(ld.scrollRate);
      return opt.has_value();
    }
  },
  { "Segments", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseInteger();
      ld.segments = opt.value_or(ld.segments);
      return opt.has_value();
    }
  },
  { "SegmentOverlapRatio", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.segmentsOverlapRatio = opt.value_or(ld.segmentsOverlapRatio);
      return opt.has_value();
    }
  },
  { "Texture", [](Objects::LaserDrawData& ld, INIFile& f) { ld.texture = f.parseString(); return !ld.texture.empty(); } },
  { "Tile", [](Objects::LaserDrawData& ld, INIFile& f) { ld.isTile = f.parseBool(); return true; } },
  { "TilingScalar", [](Objects::LaserDrawData& ld, INIFile& f) {
      auto opt = f.parseFloat();
      ld.tilingScalar = opt.value_or(ld.tilingScalar);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::Lifetime> LifetimeDataKVMap = {
  { "MaxLifetime", [](Objects::Lifetime& l, INIFile& f) {
      auto opt = f.parseInteger();
      l.maxLifetimeMs = opt.value_or(l.maxLifetimeMs);
      return opt.has_value();
    }
  },
  { "MinLifetime", [](Objects::Lifetime& l, INIFile& f) {
      auto opt = f.parseInteger();
      l.minLifetimeMs = opt.value_or(l.minLifetimeMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::LockWeapon> LockWeaponKVMap = {
  { "SlotToLock", [](Objects::LockWeapon& lw, INIFile& f) {
      auto opt = Objects::getWeaponSlot(f.parseString());
      lw.slot = opt.value_or(lw.slot);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::MaxHealthUpgrade> MaxHealthUpgradeKVMap = {
  { "AddMaxHealth", [](Objects::MaxHealthUpgrade& mh, INIFile& f) {
      auto opt = f.parseFloat();
      mh.healthUpgrade = opt.value_or(mh.healthUpgrade);
      return opt.has_value();
    }
  },
  { "ChangeType", [](Objects::MaxHealthUpgrade& mh, INIFile& f) {
      auto opt = f.parseEnum<Objects::MaxHealthModifier>(CALL(Objects::getMaxHealthModifier));
      mh.modifier = opt.value_or(mh.modifier);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::ModelConditionUpgrade> ModelConditionUpgradeKVMap = {
  { "ConditionFlag", [](Objects::ModelConditionUpgrade& mc, INIFile& f) {
      return f.parseEnumSet<Objects::ModelCondition>(mc.flags, CALL(Objects::getModelCondition));
    }
  }
};

static INIApplierMap<Objects::MissileAI> MissileAIKVMap = {
  { "DistanceToTargetBeforeDiving", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseFloat();
      md.distanceUntilDiving = opt.value_or(md.distanceUntilDiving);
      return opt.has_value();
    }
  },
  { "DistanceToTargetForLock", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseFloat();
      md.distanceUntilLock = opt.value_or(md.distanceUntilLock);
      return opt.has_value();
    }
  },
  { "DistanceToTravelBeforeTurning", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseFloat();
      md.distanceUntilReturn = opt.value_or(md.distanceUntilReturn);
      return opt.has_value();
    }
  },
  { "FuelLifetime", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseInteger();
      md.fuelLifetimeMs = opt.value_or(md.fuelLifetimeMs);
      return opt.has_value();
    }
  },
  { "IgnitionDelay", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseInteger();
      md.ignitionDelayMs = opt.value_or(md.ignitionDelayMs);
      return opt.has_value();
    }
  },
  { "InitialVelocity", [](Objects::MissileAI& md, INIFile& f) {
      auto opt = f.parseFloat();
      md.initialVelocity = opt.value_or(md.initialVelocity);
      return opt.has_value();
    }
  },
  { "TryToFollowTarget", [](Objects::MissileAI& md, INIFile& f) { md.followTarget = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::MissileLauncherBuilding> MissileLauncherBuildingKVMap = {
  { "DoorClosedFX", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorClosedEffect = f.parseString(); return !mlb.doorClosedEffect.empty(); } },
  { "DoorClosingFX", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorClosingEffect = f.parseString(); return !mlb.doorClosingEffect.empty(); } },
  { "DoorCloseTime", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) {
      auto opt = f.parseFloat();
      mlb.doorCloseTimeMs = opt.value_or(mlb.doorCloseTimeMs);
      return opt.has_value();
    }
  },
  { "DoorOpenFX", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorOpenEffect = f.parseString(); return !mlb.doorOpenEffect.empty(); } },
  { "DoorOpenIdleAudio", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorOpenIdleAudio = f.parseString(); return !mlb.doorOpenIdleAudio.empty(); } },
  { "DoorOpeningFX", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorOpeningEffect = f.parseString(); return !mlb.doorOpeningEffect.empty(); } },
  { "DoorOpenTime", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) {
      auto opt = f.parseFloat();
      mlb.doorOpenTimeMs = opt.value_or(mlb.doorOpenTimeMs);
      return opt.has_value();
    }
  },
  { "DoorWaitOpenTime", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) {
      auto opt = f.parseFloat();
      mlb.doorWaitOpenTimeMs = opt.value_or(mlb.doorWaitOpenTimeMs);
      return opt.has_value();
    }
  },
  { "DoorWaitingToCloseFX", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.doorWaitingToCloseEffect = f.parseString(); return !mlb.doorWaitingToCloseEffect.empty(); } },
  { "SpecialPowerTemplate", [](Objects::MissileLauncherBuilding& mlb, INIFile& f) { mlb.specialPower = f.parseString(); return !mlb.specialPower.empty(); } },
};

static INIApplierMap<Objects::ModelDrawData> ModelDrawDataKVMap = {
  { "AliasConditionState", [](Objects::ModelDrawData& dd, INIFile& f) {
      auto states = f.parseStringList();
      if (states.empty()) {
        return false;
      }

      std::vector<Objects::ModelCondition> conditions;
      conditions.reserve(3);

      for (auto& state : states) {
        auto opt = Objects::getModelCondition(state);
        if (!opt) {
          return false;
        }

        conditions.push_back(*opt);
      }

      dd.stateAliases.emplace_back(std::move(conditions));

      return true;
    }
  },
  { "AnimationsRequirePower", [](Objects::ModelDrawData& dd, INIFile& f) { dd.animationRequiresPower = f.parseBool(); return true; } },
  { "AttachToBoneInAnotherModule", [](Objects::ModelDrawData& dd, INIFile& f) { dd.externalBoneAttachment = f.parseString(); return !dd.externalBoneAttachment.empty(); } },
  { "ConditionState", [](Objects::ModelDrawData& dd, INIFile& f) {
      auto& state = dd.conditionStates.emplace_back();
      auto& of = static_cast<ObjectsINI&>(f);
      state.conditions = of.parseConditionStateConditions();

      if (dd.defaultConditionState.model.empty() &&
          (state.conditions.empty() || state.conditions.find(Objects::ModelCondition::NONE) != state.conditions.cend())) {
        return f.parseAttributeBlock(dd.defaultConditionState, ConditionStateKVMap);
      }

      return f.parseAttributeBlock(state, ConditionStateKVMap);
    }
  },
  { "DefaultConditionState", [](Objects::ModelDrawData& dd, INIFile& f) { return f.parseAttributeBlock(dd.defaultConditionState, ConditionStateKVMap); } },
  { "ExtraPublicBone", [](Objects::ModelDrawData& dd, INIFile& f) {
      auto value = f.parseLooseValue();
      if (value.empty()) {
        return false;
      }

      dd.extraBones.emplace_back(std::move(value));

      return true;
    }
  },
  { "IgnoreConditionStates", [](Objects::ModelDrawData& dd, INIFile& f) {
      return f.parseEnumSet<Objects::ModelCondition>(dd.ignoreConditions, CALL(Objects::getModelCondition));
    }
  },
  { "InitialRecoilSpeed", [](Objects::ModelDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.initialRecoilSpeed = value.value_or(t.initialRecoilSpeed);
      return value.has_value();
    }
  },
  { "MaxRecoilDistance", [](Objects::ModelDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.maxRecoilDistance = value.value_or(t.maxRecoilDistance);
      return value.has_value();
    }
  },
  { "MinLODRequired", [](Objects::ModelDrawData& /*t*/, INIFile& f) { f.parseString(); /* ignore */ return true; } },
  { "OkToChangeModelColor", [](Objects::ModelDrawData& dd, INIFile& f) { dd.canChangeColor = f.parseBool(); return true; } },
  { "ParticlesAttachedToAnimatedBones", [](Objects::ModelDrawData& dd, INIFile& f) { dd.animatedParticles = f.parseBool(); return true; } },
  { "ProjectileBoneFeedbackEnabledSlots", [](Objects::ModelDrawData& dd, INIFile& f) {
      return f.parseEnumSet<Objects::WeaponSlot>(dd.feedbackSlots, CALL(Objects::getWeaponSlot));
    }
  },
  { "ReceivesDynamicLights", [](Objects::ModelDrawData& dd, INIFile& f) { dd.dynamicIllumination = f.parseBool(); return true; } },
  { "RecoilSettleSpeed", [](Objects::ModelDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.recoilSettleSpeed = value.value_or(t.recoilSettleSpeed);
      return value.has_value();
    }
  },
  { "TransitionState", [](Objects::ModelDrawData& dd, INIFile& f) {
      auto& state = dd.transitionStates.emplace_back();
      auto transition = f.parseStringList();
      // There may be more (3), like: UP_SNOW NIGHT DOWN_DEFAULT
      if (transition.size() < 2) {
        return false;
      }

      state.isTransition = true;
      state.transitionFromTo =
        std::make_pair(std::move(transition[0]), std::move(transition[1]));

      return f.parseAttributeBlock(state, ConditionStateKVMap);
    }
  },
  { "TrackMarks", [](Objects::ModelDrawData& dd, INIFile& f) { dd.trackMarksTexture = f.parseString(); return !dd.trackMarksTexture.empty(); } },
};

static INIApplierMap<Objects::DependencyModelDrawData> DependencyModelDrawDataKVMap = {
  { "AttachToBoneInContainer", [](Objects::DependencyModelDrawData& dd, INIFile& f) { dd.attachTo = f.parseString(); return !dd.attachTo.empty(); } },
};

static INIApplierMap<Objects::ObjectCreationUpgrade> ObjectCreationUpgradeKVMap = {
  { "UpgradeObject", [](Objects::ObjectCreationUpgrade& ou, INIFile& f) { ou.object = f.parseString(); return !ou.object.empty(); } },
};

static INIApplierMap<Objects::OCL> OCLKVMap = {
  { "CreateAtEdge", [](Objects::OCL& ocl, INIFile& f) { ocl.createAtEdge = f.parseBool(); return true; } },
  { "FactionOCL", [](Objects::OCL& ocl, INIFile& f) {
      auto values = f.parseAttributes();
      if (values.size() != 2) {
        return false;
      }

      Objects::FactionOCL fOCL;
      if (values.contains("OCL")) {
        fOCL.ocl = std::move(values["OCL"]);
      } else {
        return false;
      }

      if (values.contains("Faction")) {
        fOCL.faction = std::move(values["Faction"]);
      } else {
        return false;
      }

      ocl.factionOCLs.emplace_back(std::move(fOCL));

      return true;
    }
  },
  { "FactionTriggered", [](Objects::OCL& ocl, INIFile& f) { ocl.factionTriggered = f.parseBool(); return true; } },
  { "MaxDelay", [](Objects::OCL& ocl, INIFile& f) {
      auto opt = f.parseInteger();
      ocl.maxDelayMs = opt.value_or(ocl.maxDelayMs);
      return opt.has_value();
    }
  },
  { "MinDelay", [](Objects::OCL& ocl, INIFile& f) {
      auto opt = f.parseInteger();
      ocl.minDelayMs = opt.value_or(ocl.minDelayMs);
      return opt.has_value();
    }
  },
  { "OCL", [](Objects::OCL& ocl, INIFile& f) { ocl.ocl = f.parseString(); return !ocl.ocl.empty(); } },
};

static INIApplierMap<Objects::OCLSpecialPower> OCLSpecialPowerKVMap = {
  { "CreateLocation", [](Objects::OCLSpecialPower& osp, INIFile& f) {
      auto opt = f.parseEnum<Objects::OCLLocation>(CALL(Objects::getOCLLocation));
      osp.location = opt.value_or(osp.location);
      return opt.has_value();
    }
  },
  { "OCL", [](Objects::OCLSpecialPower& osp, INIFile& f) {
      auto value = f.parseString();
      if (value.empty()) {
        return false;
      }

      osp.OCLs.push_back(std::move(value));

      return true;
    }
  },
  { "OCLAdjustPositionToPassable", [](Objects::OCLSpecialPower& osp, INIFile& f) { osp.adjustToNextPassable = f.parseBool(); return true; } },
  { "UpgradeOCL", [](Objects::OCLSpecialPower& osp, INIFile& f) {
      auto value = f.parseStringList();
      if (value.size() < 2) {
        return false;
      }

      osp.upgradeOCLs.push_back(std::make_pair(std::move(value[0]), std::move(value[1])));

      return true;
    }
  },
  { "ReferenceObject", [](Objects::OCLSpecialPower& osp, INIFile& f) { osp.reference = f.parseString(); return !osp.reference.empty(); } },
};

static INIApplierMap<Objects::OverlordContain> OverlordContainKVMap = {
  { "ExperienceSinkForRider", [](Objects::OverlordContain& oc, INIFile& f) { oc.xpForRider = f.parseBool(); return true; } },
  { "PayloadTemplateName", [](Objects::OverlordContain& oc, INIFile& f) { oc.payload = f.parseString(); return !oc.payload.empty(); } },
};

static INIApplierMap<Objects::Overcharge> OverchargeKVMap = {
  { "HealthPercentToDrainPerSecond", [](Objects::Overcharge& oc, INIFile& f) {
      auto opt = f.parsePercent();
      oc.healthDrainPerSecond = opt.value_or(oc.healthDrainPerSecond);
      return opt.has_value();
    }
  },
  { "NotAllowedWhenHealthBelowPercent", [](Objects::Overcharge& oc, INIFile& f) {
      auto opt = f.parsePercent();
      oc.minRequiredHealth = opt.value_or(oc.minRequiredHealth);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::ParkingPlace> ParkingPlaceKVMap = {
  { "ApproachHeight", [](Objects::ParkingPlace& pp, INIFile& f) {
      auto opt = f.parseFloat();
      pp.approachHeight = opt.value_or(pp.approachHeight);
      return opt.has_value();
    }
  },
  { "HasRunways", [](Objects::ParkingPlace& pp, INIFile& f) { pp.hasRunways = f.parseBool(); return true; } },
  { "HealAmountPerSecond", [](Objects::ParkingPlace& pp, INIFile& f) {
      auto opt = f.parseFloat();
      pp.healingPerSecond = opt.value_or(pp.healingPerSecond);
      return opt.has_value();
    }
  },
  { "NumCols", [](Objects::ParkingPlace& pp, INIFile& f) {
      auto opt = f.parseSignedInteger();
      pp.numCols = opt.value_or(pp.numCols);
      return opt.has_value();
    }
  },
  { "NumRows", [](Objects::ParkingPlace& pp, INIFile& f) {
      auto opt = f.parseSignedInteger();
      pp.numRows = opt.value_or(pp.numRows);
      return opt.has_value();
    }
  },
  { "ParkInHangars", [](Objects::ParkingPlace& pp, INIFile& f) { pp.inHangars = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::ParticleUplinkCannon> ParticleUplinkCannonKVMap = {
  { "*", SKIP(Objects::ParticleUplinkCannon) }
};

static INIApplierMap<Objects::OpenContain> OpenContainKVMap = {
  { "AllowAlliesInside", [](Objects::OpenContain& oc, INIFile& f) { oc.allowAllies = f.parseBool(); return true; } },
  { "AllowEnemiesInside", [](Objects::OpenContain& oc, INIFile& f) { oc.allowEnemies = f.parseBool(); return true; } },
  { "AllowInsideKindOf", [](Objects::OpenContain& oc, INIFile& f) { return f.parseEnumSet<Objects::Attribute>(oc.guestInclusion, CALL(Objects::getAttribute)); } },
  { "AllowNeutralInside", [](Objects::OpenContain& oc, INIFile& f) { oc.allowNeutrals = f.parseBool(); return true; } },
  { "ContainMax", [](Objects::OpenContain& oc, INIFile& f) {
      auto opt = f.parseSignedInteger();
      oc.max = opt.value_or(oc.max);
      return opt.has_value();
    }
  },
  { "EnterSound", [](Objects::OpenContain& oc, INIFile& f) { oc.enterSound = f.parseString(); return !oc.enterSound.empty(); } },
  { "ExitSound", [](Objects::OpenContain& oc, INIFile& f) { oc.exitSound = f.parseString(); return !oc.exitSound.empty(); } },
  { "ForbidInsideKindOf", [](Objects::OpenContain& oc, INIFile& f) { return f.parseEnumSet<Objects::Attribute>(oc.guestExclusion, CALL(Objects::getAttribute)); } },
  { "DamagePercentToUnits", [](Objects::OpenContain& oc, INIFile& f) {
      auto opt = f.parsePercent();
      oc.damageToUnits = opt.value_or(oc.damageToUnits);
      return opt.has_value();
    }
  },
  { "DoorOpenTime", [](Objects::OpenContain& oc, INIFile& f) {
      auto opt = f.parseInteger();
      oc.doorOpenTimeMs = opt.value_or(oc.doorOpenTimeMs);
      return opt.has_value();
    }
  },
  { "NumberOfExitPaths", [](Objects::OpenContain& oc, INIFile& f) {
      auto opt = f.parseSignedInteger();
      oc.numExitPaths = opt.value_or(oc.numExitPaths);
      return opt.has_value();
    }
  },
  { "PassengersAllowedToFire", [](Objects::OpenContain& oc, INIFile& f) { oc.unitsCanFire = f.parseBool(); return true; } },
  { "PassengersInTurret", [](Objects::OpenContain& oc, INIFile& f) { oc.unitsInTurret = f.parseBool(); return true; } }
};

// TODO
static INIApplierMap<Objects::ParachuteContain> ParachuteContainKVMap = {
  { "*", SKIP(Objects::ParachuteContain) }
};

// TODO
static INIApplierMap<Objects::PilotFindVehicle> PilotFindVehicleKVMap = {
  { "*", SKIP(Objects::PilotFindVehicle) }
};

static INIApplierMap<Objects::Physics> PhysicsDataKVMap = {
  { "AerodynamicFriction", [](Objects::Physics& p, INIFile& f) {
        auto opt = f.parseFloat();
        p.aerodynamicFriction = opt.value_or(p.aerodynamicFriction);
        return opt.has_value();
      }
  },
  { "AllowBouncing", [](Objects::Physics& p, INIFile& f) { p.bouncing = f.parseBool(); return true; } },
  { "CenterOfMassOffset", [](Objects::Physics& p, INIFile& f) {
      auto opt = f.parseFloat();
      p.massCenterOffset = opt.value_or(p.massCenterOffset);
      return opt.has_value();
    }
  },
  { "ForwardFriction", [](Objects::Physics& p, INIFile& f) {
        auto opt = f.parseFloat();
        p.forwardFriction = opt.value_or(p.forwardFriction);
        return opt.has_value();
      }
  },
  { "KillWhenRestingOnGround", [](Objects::Physics& p, INIFile& f) { p.killOnGround = f.parseBool(); return true; } },
  { "Mass", [](Objects::Physics& p, INIFile& f) {
      auto opt = f.parseFloat();
      p.mass = opt.value_or(p.mass);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::PointDefenseLaser> PointDefenseLaserKVMap = {
  { "PrimaryTargetTypes", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(pdl.primaryTargets, CALL(Objects::getAttribute));
    }
  },
  { "PredictTargetVelocityFactor", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      auto opt = f.parseFloat();
      pdl.velocityFactor = opt.value_or(pdl.velocityFactor);
      return opt.has_value();
    }
  },
  { "ScanRange", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      auto opt = f.parseFloat();
      pdl.scanRange = opt.value_or(pdl.scanRange);
      return opt.has_value();
    }
  },
  { "ScanRate", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      auto opt = f.parseInteger();
      pdl.scanRate = opt.value_or(pdl.scanRate);
      return opt.has_value();
    }
  },
  { "SecondaryTargetTypes", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(pdl.secondaryTargets, CALL(Objects::getAttribute));
    }
  },
  { "WeaponTemplate", [](Objects::PointDefenseLaser& pdl, INIFile& f) {
      pdl.weapon = f.parseString();
      return !pdl.weapon.empty();
    }
  }
};

static INIApplierMap<Objects::Poisoned> PoisonedKVMap = {
  { "PoisonDamageInterval", [](Objects::Poisoned& pd, INIFile& f) {
      auto opt = f.parseInteger();
      pd.intervalMs = opt.value_or(pd.intervalMs);
      return opt.has_value();
    }
  },
  { "PoisonDuration", [](Objects::Poisoned& pd, INIFile& f) {
      auto opt = f.parseInteger();
      pd.durationMs = opt.value_or(pd.durationMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::PowerPlant> PowerPlantKVMap = {
  { "RodsExtendTime", [](Objects::PowerPlant& pp, INIFile& f) {
      auto opt = f.parseInteger();
      pp.rodsExtendTimeMs = opt.value_or(pp.rodsExtendTimeMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::Production> ProductionKVMap = {
  { "ConstructionCompleteDuration", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseInteger();
      p.constructionTimeMs = opt.value_or(p.constructionTimeMs);
      return opt.has_value();
    }
  },
  { "DisabledTypesToProcess", [](Objects::Production& p, INIFile& f) {
      return f.parseEnumSet<Objects::DisabledType>(p.disabledTypes, CALL(Objects::getDisabledType));
    }
  },
  { "DoorCloseTime", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseInteger();
      p.doorClosingTimeMs = opt.value_or(p.doorClosingTimeMs);
      return opt.has_value();
    }
  },
  { "DoorOpeningTime", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseInteger();
      p.doorOpeningTimeMs = opt.value_or(p.doorOpeningTimeMs);
      return opt.has_value();
    }
  },
  { "DoorWaitOpenTime", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseInteger();
      p.doorWaitOpenTimeMs = opt.value_or(p.doorWaitOpenTimeMs);
      return opt.has_value();
    }
  },
  { "MaxQueueEntries", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseSignedInteger();
      p.maxQueue = opt.value_or(p.maxQueue);
      return opt.has_value();
    }
  },
  { "NumDoorAnimations", [](Objects::Production& p, INIFile& f) {
      auto opt = f.parseSignedInteger();
      p.numDoorAnimations = opt.value_or(p.numDoorAnimations);
      return opt.has_value();
    }
  },
  { "QuantityModifier", [](Objects::Production& p, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      auto qtyOpt = f.parseInteger(values[1]);
      if (!qtyOpt) {
        return false;
      }

      p.qtyModifiers.emplace_back(std::make_pair(std::move(values[0]), *qtyOpt));

      return true;
    }
  }
};

static INIApplierMap<Objects::PropagandaTower> PropagandaTowerKVMap = {
  { "DelayBetweenUpdates", [](Objects::PropagandaTower& pt, INIFile& f) {
      auto opt = f.parseInteger();
      pt.scanDelayFrames = opt.value_or(pt.scanDelayFrames);
      return opt.has_value();
    }
  },
  { "HealPercentEachSecond", [](Objects::PropagandaTower& pt, INIFile& f) {
      auto opt = f.parsePercent();
      pt.autoHealPercentPerSec = opt.value_or(pt.autoHealPercentPerSec);
      return opt.has_value();
    }
  },
  { "PulseFX", [](Objects::PropagandaTower& pt, INIFile& f) { pt.pulseEffect = f.parseString(); return !pt.pulseEffect.empty(); } },
  { "Radius", [](Objects::PropagandaTower& pt, INIFile& f) {
      auto opt = f.parseFloat();
      pt.scanRadius = opt.value_or(pt.scanRadius);
      return opt.has_value();
    }
  },
  { "UpgradedHealPercentEachSecond", [](Objects::PropagandaTower& pt, INIFile& f) {
      auto opt = f.parsePercent();
      pt.upgradedAutoHealPercentPerSec = opt.value_or(pt.upgradedAutoHealPercentPerSec);
      return opt.has_value();
    }
  },
  { "UpgradedPulseFX", [](Objects::PropagandaTower& pt, INIFile& f) { pt.upgradedPulseEffect = f.parseString(); return !pt.upgradedPulseEffect.empty(); } },
  { "UpgradeRequired", [](Objects::PropagandaTower& pt, INIFile& f) { pt.requiredUpgrade = f.parseString(); return !pt.requiredUpgrade.empty(); } }
};

static INIApplierMap<Objects::ObjectBuilder> PrerequisitesKVMap = {
  { "Object", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto values = f.parseStringList();
      if (values.empty()) {
        return false;
      }

      b.objectPrerequisites.insert(
          b.objectPrerequisites.end()
        , values.cbegin()
        , values.cend()
      );

      return true;
    }
  },
  { "Science", [](Objects::ObjectBuilder& b, INIFile& f) { b.sciencePrerequisite = f.parseString(); return !b.sciencePrerequisite.empty(); } }
};

static INIApplierMap<Objects::RepairDock> RepairDockKVMap = {
  { "TimeForFullHeal", [](Objects::RepairDock& rd, INIFile& f) {
      auto value = f.parseInteger();
      rd.timeToHealMs = value.value_or(rd.timeToHealMs);
      return value.has_value();
    }
  }
};

static INIApplierMap<Objects::ReplaceObjectUpgrade> ReplaceObjectUpgradeKVMap = {
  { "ReplaceObject", [](Objects::ReplaceObjectUpgrade& up, INIFile& f) { up.object = f.parseString(); return !up.object.empty(); } }
};

static INIApplierMap<Objects::TransportContain> TransportContainKVMap = {
  { "ArmedRidersUpgradeMyWeaponSet", [](Objects::TransportContain& tc, INIFile& f) { tc.armedRidersWeaponUpgrade = f.parseBool(); return true; } },
  { "DestroyRidersWhoAreNotFreeToExit", [](Objects::TransportContain& tc, INIFile& f) { tc.destroyTrappedRiders = f.parseBool(); return true; } },
  { "ExitBone", [](Objects::TransportContain& tc, INIFile& f) {
      auto value = f.parseString();
      tc.exitBone = value;
      return !tc.exitBone.empty();
    }
  },
  { "ExitDelay", [](Objects::TransportContain& tc, INIFile& f) {
      auto opt = f.parseInteger();
      tc.exitDelayMs = opt.value_or(tc.exitDelayMs);
      return opt.has_value();
    }
  },
  { "ExitPitchRate", [](Objects::TransportContain& tc, INIFile& f) {
      auto opt = f.parseFloat();
      tc.exitPitchRate = opt.value_or(tc.exitPitchRate);
      return opt.has_value();
    }
  },
  { "GoAggressiveOnExit", [](Objects::TransportContain& tc, INIFile& f) { tc.exitAggressively = f.parseBool(); return true; } },
  { "HealthRegen%PerSec", [](Objects::TransportContain& tc, INIFile& f) {
      auto opt = f.parseFloat();
      tc.healthRegenPerSecond = opt.value_or(tc.healthRegenPerSecond);
      return opt.has_value();
    }
  },
  { "InitialPayload", [](Objects::TransportContain& tc, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() != 2) {
        return false;
      }

      auto opt = f.parseInteger(values[1]);
      if (!opt) {
        return false;
      }

      tc.initialPayload.first = values[0];
      tc.initialPayload.second = *opt;

      return true;
    }
  },
  { "KeepContainerVelocityOnExit", [](Objects::TransportContain& tc, INIFile& f) { tc.exitContainerKeepSpeed = f.parseBool(); return true; } },
  { "OrientLikeContainerOnExit", [](Objects::TransportContain& tc, INIFile& f) { tc.exitOrientationAsContainer = f.parseBool(); return true; } },
  { "ScatterNearbyOnExit", [](Objects::TransportContain& tc, INIFile& f) { tc.exitScattering = f.parseBool(); return true; } },
  { "Slots", [](Objects::TransportContain& tc, INIFile& f) {
      auto opt = f.parseInteger();
      tc.slots = opt.value_or(tc.slots);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::TunnelContain> TunnelContainKVMap = {
  { "TimeForFullHeal", [](Objects::TunnelContain& tc, INIFile& f) {
      auto opt = f.parseInteger();
      tc.timeFullHealMs = opt.value_or(tc.timeFullHealMs);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::SupplyTruckAI> SupplyTruckAIKVMap = {
  { "MaxBoxes", [](Objects::SupplyTruckAI& ai, INIFile& f) {
      auto opt = f.parseInteger();
      ai.maxBoxes = opt.value_or(ai.maxBoxes);
      return opt.has_value();
    }
  },
  { "SuppliesDepletedVoice", [](Objects::SupplyTruckAI& ai, INIFile& f) { ai.depletedSound = f.parseString(); return !ai.depletedSound.empty(); } },
  { "SupplyCenterActionDelay", [](Objects::SupplyTruckAI& ai, INIFile& f) {
      auto opt = f.parseInteger();
      ai.supplyCenterDelayMs = opt.value_or(ai.supplyCenterDelayMs);
      return opt.has_value();
    }
  },
  { "SupplyWarehouseActionDelay", [](Objects::SupplyTruckAI& ai, INIFile& f) {
      auto opt = f.parseInteger();
      ai.warehouseDelayMs = opt.value_or(ai.warehouseDelayMs);
      return opt.has_value();
    }
  },
  { "SupplyWarehouseScanDistance", [](Objects::SupplyTruckAI& ai, INIFile& f) {
      auto opt = f.parseFloat();
      ai.warehouseScanDistance = opt.value_or(ai.warehouseScanDistance);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::SupplyWarehouseCrippling> SupplyWarehouseCripplingKVMap = {
  { "SelfHealAmount", [](Objects::SupplyWarehouseCrippling& swc, INIFile& f) {
      auto opt = f.parseInteger();
      swc.healAmount = opt.value_or(swc.healAmount);
      return opt.has_value();
    }
  },
  { "SelfHealDelay", [](Objects::SupplyWarehouseCrippling& swc, INIFile& f) {
      auto opt = f.parseInteger();
      swc.healIntervalMs = opt.value_or(swc.healIntervalMs);
      return opt.has_value();
    }
  },
  { "SelfHealSupression", [](Objects::SupplyWarehouseCrippling& swc, INIFile& f) {
      auto opt = f.parseInteger();
      swc.healSuppressionMs = opt.value_or(swc.healSuppressionMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::SupplyWarehouseDock> SupplyWarehouseDockKVMap = {
  { "DeleteWhenEmpty", [](Objects::SupplyWarehouseDock& swd, INIFile& f) { swd.deleteWhenEmpty = f.parseBool(); return true; } },
  { "StartingBoxes", [](Objects::SupplyWarehouseDock& swd, INIFile& f) {
      auto opt = f.parseInteger();
      swd.numBoxes = opt.value_or(swd.numBoxes);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::TensileFormation> TensileFormationKVMap = {
  { "Enabled", [](Objects::TensileFormation& tf, INIFile& f) { tf.enabled = f.parseBool(); return true; } },
  { "CrackSound", [](Objects::TensileFormation& tf, INIFile& f) { tf.crackSound = f.parseString(); return !tf.crackSound.empty(); } }
};

static bool parseTransitionDamageFX(INIFile& f, Objects::TransitionDamageFXSlots slots, size_t damageTypeIdx, size_t effectIdx) {
  auto values = f.parseAttributes();
  auto& damage = slots[damageTypeIdx][effectIdx];

  if (values.size() < 4) {
    return false;
  }

  if (values.contains("X")) {
    damage.location.x = f.parseFloat(values["X"]).value_or(0.0f);
  }
  if (values.contains("Y")) {
    damage.location.z = f.parseFloat(values["Y"]).value_or(0.0f);
  }
  if (values.contains("Z")) {
    damage.location.y = f.parseFloat(values["Z"]).value_or(0.0f);
  }
  if (values.contains("FXList")) {
    damage.effect = values["FXList"];
  }

  return true;
}

static bool parseTransitionDamageParticles(INIFile& f, Objects::TransitionDamageParticlesSlots slots, size_t damageTypeIdx, size_t effectIdx) {
  auto values = f.parseAttributes();
  auto& damage = slots[damageTypeIdx][effectIdx];

  if (values.size() < 3) {
    return false;
  }

  if (values.contains("Bone")) {
    damage.bone = values["Bone"];
  }
  if (values.contains("Psys")) {
    damage.particleSystem = values["Psys"];
  }
  if (values.contains("RandomBone")) {
    damage.randomBone = f.parseBool(values["RandomBone"]);
  }

  return true;
};

static INIApplierMap<Objects::TransitionDamageFX> TransitionDamageFXKVMap = {
  { "DamageFXTypes", [](Objects::TransitionDamageFX& fx, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(fx.damageEffectTypes, CALL(Objects::getDamageType));
    }
  },
  { "DamagedFXList1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageFX(f, fx.effects, 1, 0); } },
  { "DamageParticleTypes", [](Objects::TransitionDamageFX& fx, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(fx.damageParticleTypes, CALL(Objects::getDamageType));
    }
  },
  { "DamagedParticleSystem1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 1, 0); } },
  { "DamagedParticleSystem4", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 1, 3); } },
  { "DamagedParticleSystem5", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 1, 4); } },
  { "DamagedParticleSystem6", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 1, 5); } },
  { "ReallyDamagedFXList1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageFX(f, fx.effects, 2, 0); } },
  { "ReallyDamagedParticleSystem1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 0); } },
  { "ReallyDamagedParticleSystem2", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 1); } },
  { "ReallyDamagedParticleSystem3", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 2); } },
  { "ReallyDamagedParticleSystem4", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 3); } },
  { "ReallyDamagedParticleSystem5", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 4); } },
  { "ReallyDamagedParticleSystem6", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 5); } },
  { "ReallyDamagedParticleSystem7", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 6); } },
  { "ReallyDamagedParticleSystem8", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 2, 7); } },
  { "RubbleFXList1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageFX(f, fx.effects, 3, 0); } },
  { "RubbleParticleSystem1", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 0); } },
  { "RubbleParticleSystem2", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 1); } },
  { "RubbleParticleSystem3", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 2); } },
  { "RubbleParticleSystem4", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 3); } },
  { "RubbleParticleSystem5", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 4); } },
  { "RubbleParticleSystem6", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 5); } },
  { "RubbleParticleSystem7", [](Objects::TransitionDamageFX& fx, INIFile& f) { return parseTransitionDamageParticles(f, fx.particleSystems, 3, 6); } },
};

static bool parseSound(Objects::ObjectBuilder& b, INIFile& f, Objects::Noise noise) {
  auto string = f.parseString();
  if (string.empty()) {
    return false;
  }

  if (string == "NoSound") {
    return true;
  }

  b.noises.emplace(
    std::make_pair<Objects::Noise, std::string>(
        std::move(noise)
      , std::move(string)
    )
  );

  return true;
};

static INIApplierMap<Objects::ObjectBuilder> UnitSpecificFXKV = {
  { "CombatDropKillFX", [](Objects::ObjectBuilder& b, INIFile& f) { b.unitCombatDropKillEffect = f.parseString(); return !b.unitCombatDropKillEffect.empty(); } }
};

static INIApplierMap<Objects::ObjectBuilder> UnitSpecificSoundsKV = {
  { "Afterburner", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_AFTERBURNER); } },
  { "Deploy", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_DEPLOY); } },
  { "HowitzerFire", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_HOWITZER_FIRE); } },
  { "SoundEject", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_EJECT); } },
  { "StartDive", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_START_DIVE); } },
  { "TruckLandingSound", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_TRUCK_LANDING_SOUND); } },
  { "TruckPowerslideSound", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_TRUCK_POWERSLIDE_SOUND); } },
  { "TurretMoveStart", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_TURRET_MOVE_START); } },
  { "TurretMoveLoop", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_TURRET_MOVE_LOOP); } },
  { "Undeploy", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_UNDEPLOY); } },
  { "UnderConstruction", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_UNDER_CONSTRUCTION); } },
  { "VoiceBombard", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_BOMBARD); } },
  { "VoiceBuildResponse", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_BUILD_RESPONSE); } },
  { "VoiceCreate", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_CREATE); } },
  { "VoiceClearBuilding", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_CLEAR_BUILDING); } },
  { "VoiceCombatDrop", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_COMBAT_DROP); } },
  { "VoiceCreate", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_CREATE); } },
  { "VoiceCrush", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_CRUSH); } },
  { "VoiceDisarm", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_DISARM); } },
  { "VoiceEject", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_EJECT); } },
  { "VoiceEnter", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_ENTER); } },
  { "VoiceEnterHostile", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_ENTER_HOSTILE); } },
  { "VoiceFlameLocation", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_FLAME_LOCATION); } },
  { "VoiceFireRocketPods", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_FIRE_ROCKET_PODS); } },
  { "VoiceGarrison", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_GARRISON); } },
  { "VoiceGetHealed", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_GET_HEALED); } },
  { "VoiceLowFuel", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_LOW_FUEL); } },
  { "VoiceMelee", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_MELEE); } },
  { "VoiceNoBuild", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_NO_BUILD); } },
  { "VoicePrimaryWeaponMode", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_PRIMARY_WEAPON_MODE); } },
  { "VoiceRapidFire", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_RAPID_FIRE); } },
  { "VoiceRepair", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_REPAIR); } },
  { "VoiceSalvage", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_SALVAGE); } },
  { "VoiceSecondaryWeaponMode", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_SECONDARY_WEAPON_MODE); } },
  { "VoiceSubdue", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_SUBDUE); } },
  { "VoiceSupply", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_SUPPLY); } },
  { "VoiceUnload", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_UNLOAD); } },
};

static INIApplierMap<Objects::UpgradeDie> UpgradeDieKVMap = {
  { "UpgradeToRemove", [](Objects::UpgradeDie& upg, INIFile& f) {
      auto values = f.parseStringList();
      // Trailing garbage
      if (values.empty()) {
        return false;
      }

      upg.removeUpgrade = values[0];
      return true;
    }
  }
};

static INIApplierMap<Objects::UnpauseSpecialPowerUpgrade> UnpauseSpecialPowerUpgradeKVMap = {
  { "SpecialPowerTemplate", [](Objects::UnpauseSpecialPowerUpgrade& upg, INIFile& f) {
      upg.specialPower = f.parseString();
      return !upg.specialPower.empty();
    }
  }
};

template<typename T>
static INIApplierMap<T> UpgradeKVMap = {
  { "ConflictsWith", [](T& upg, INIFile& f) {
      auto values = f.parseStringList();
      if (values.empty()) {
        return false;
      }

      upg.conflicts.insert(upg.conflicts.end(), values.cbegin(), values.cend());

      return true;
    }
  },
  { "TriggeredBy", [](T& upg, INIFile& f) {
      auto string = f.parseString();
      if (string.empty()) {
        return false;
      }

      upg.triggers.emplace_back(std::move(string));

      return true;
    }
  }
};

static std::vector<Objects::Locomotor> parseLocomotors(INIFile& f) {
  auto values = f.parseStringList();
  if (values.size() < 2) {
    return {};
  }

  auto opt = Objects::getLocomotorType(values[0]);
  if (!opt) {
    return {};
  }

  auto type = *opt;
  std::vector<Objects::Locomotor> locomotors;
  locomotors.reserve(values.size() - 1);

  for (size_t i = 1; i < values.size(); ++i) {
    auto& locomotor = locomotors.emplace_back();
    locomotor.locomotor = std::move(values[i]);
  }

  return locomotors;
};

static INIApplierMap<Objects::WeaponSet> WeaponSetKVMap = {
  { "AutoChooseSources", [](Objects::WeaponSet& ws, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      auto weaponSlot = Objects::getWeaponSlot(values[0]);
      if (!weaponSlot) {
        return false;
      }

      std::set<Objects::CommandSource> byCommands;
      for (size_t i = 1; i < values.size(); ++i) {
        auto value = Objects::getCommandSource(values[i]);
        if (!value) {
          return false;
        }

        byCommands.insert(*value);
      }

      auto& slot = ws.weapons[weaponSlotToIdx(*weaponSlot)];
      slot.slot = *weaponSlot;
      slot.sources = std::move(byCommands);

      return true;
    }
  },
  { "Conditions", [](Objects::WeaponSet& ws, INIFile& f) { return f.parseEnumSet<Objects::WeaponSet::Condition>(ws.conditions, CALL(Objects::getWeaponSetCondition)); } },
  { "PreferredAgainst", [](Objects::WeaponSet& ws, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      auto weaponSlot = Objects::getWeaponSlot(values[0]);
      if (!weaponSlot) {
        return false;
      }

      std::set<Objects::Attribute> against;
      for (size_t i = 1; i < values.size(); ++i) {
        auto value = Objects::getAttribute(values[i]);
        if (!value) {
          return false;
        }

        against.insert(*value);
      }

      auto& slot = ws.weapons[weaponSlotToIdx(*weaponSlot)];
      slot.slot = *weaponSlot;
      slot.useAgainst = std::move(against);

      return true;
    }
  },
  { "ShareWeaponReloadTime", [](Objects::WeaponSet& ws, INIFile& f) { ws.sharedReloadTime = f.parseBool(); return true;  } },
  { "Weapon", [](Objects::WeaponSet& ws, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() < 2) {
        return false;
      }

      auto weaponSlot = Objects::getWeaponSlot(values[0]);
      if (!weaponSlot) {
        return false;
      }

      auto& slot = ws.weapons[weaponSlotToIdx(*weaponSlot)];
      slot.slot = *weaponSlot;
      slot.name = std::move(values[1]);

      return true;
    }
  },
  { "WeaponLockSharedAcrossSets", [](Objects::WeaponSet& ws, INIFile& f) {
      ws.sharedLock = f.parseBool();
      return true;
    }
  }
};

static std::optional<std::array<uint16_t, 4>> parseExperience(INIFile& f) {
  std::array<uint16_t, 4> values;
  auto stringValues = f.parseStringList();
  if (stringValues.size() < 4) {
    return {};
  }

  // There may be five values, assume first one is 0 for now
  uint8_t offset = 0;
  if (stringValues.size() > 4) {
    offset = 1;
  }

  for (uint8_t i = 0; i < 4; ++i) {
    auto opt = f.parseSignedShort(stringValues[offset + i]);
    if (!opt) {
      return {};
    }

    values[i] = *opt;
  }

  return {values};
};

static INIApplierMap<Objects::ObjectBuilder> ObjectDataKVMap = {
  { "ArmorSet", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto& armorSet = b.armorSets.emplace_back();
      return f.parseAttributeBlock(armorSet, ArmorSetKVMap);
    }
  },
  { "Behavior", [](Objects::ObjectBuilder& b, INIFile& f) { return reinterpret_cast<ObjectsINI&>(f).parseBehavior(b); } },
  { "Body", [](Objects::ObjectBuilder& b, INIFile& f) { return reinterpret_cast<ObjectsINI&>(f).parseBody(b); } },
  { "BuildCost", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseShort();
      b.buildCost = opt.value_or(b.buildCost);
      return opt.has_value();
    }
  },
  { "BuildCompletion", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseString();
      if (value.empty()) {
        return false;
      }

      auto opt = Objects::getCompletionAppearance(value);
      b.completionAppearance = opt.value_or(b.completionAppearance);
      return opt.has_value();
    }
  },
  { "BuildTime", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseShort();
      b.buildTimeSec = opt.value_or(b.buildTimeSec);
      return opt.has_value();
    }
  },
  { "ButtonImage", [](Objects::ObjectBuilder& b, INIFile& f) { b.buttonImage = f.parseString(); return !b.buttonImage.empty(); } },
  { "ClientUpdate", [](Objects::ObjectBuilder& b, INIFile& f) { return reinterpret_cast<ObjectsINI&>(f).parseClientUpdate(b); } },
  { "CommandSet", [](Objects::ObjectBuilder& b, INIFile& f) { b.commandSet = f.parseString(); return !b.commandSet.empty(); } },
  { "CrushableLevel", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseByte();
      b.crushableLevel = value.value_or(b.crushableLevel);
      return value.has_value();
    }
  },
  { "CrusherLevel", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseByte();
      b.crushingLevel = value.value_or(b.crushingLevel);
      return value.has_value();
    }
  },
  { "DisplayName", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto values = f.parseStringList();
      if (values.empty()) {
        return false;
      }

      // TODO: translate
      b.displayName = u"TODO";
      return true;
    }
  },
  { "Draw", [](Objects::ObjectBuilder& b, INIFile& f) { return reinterpret_cast<ObjectsINI&>(f).parseDraw(b); } },
  { "EditorSorting", [](Objects::ObjectBuilder& /*b*/, INIFile& f) { f.parseStringList(); return true; } },
  { "EnergyBonus", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseSignedInteger();
      b.energyBonus = opt.value_or(b.energyBonus);
      return opt.has_value();
    }
  },
  { "EnergyProduction", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseSignedInteger();
      b.energyContribution = opt.value_or(b.energyContribution);
      return opt.has_value();
    }
  },
  { "ExperienceRequired", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = parseExperience(f);
      if (!opt) {
        return false;
      }

      b.experienceRequirements = *opt;

      return true;
    }
  },
  { "ExperienceValue", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = parseExperience(f);
      if (!opt) {
        return false;
      }

      b.experienceValues = *opt;

      return true;
    }
  },
  { "FactoryExitWidth", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.factoryExitWidth = value.value_or(b.factoryExitWidth);
      return value.has_value();
    }
  },
  { "FactoryExtraBibWidth", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.factoryExtraBibWidth = value.value_or(b.factoryExtraBibWidth);
      return value.has_value();
    }
  },
  { "FenceWidth", [](Objects::ObjectBuilder& /*b*/, INIFile& f) { f.parseFloat(); return true; } },
  { "FenceXOffset", [](Objects::ObjectBuilder& /*b*/, INIFile& f) { f.parseFloat(); return true; } },
  { "Geometry", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseEnum<Objects::Geometry>(CALL(Objects::getGeometry));
      b.geometry.type = opt.value_or(Objects::Geometry::NONE);
      return opt.has_value();
    }
  },
  { "GeometryMajorRadius", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.majorRadius = value.value_or(b.geometry.majorRadius);
      return value.has_value();
    }
  },
  { "GeometryMinorRadius", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.minorRadius = value.value_or(b.geometry.minorRadius);
      return value.has_value();
    }
  },
  { "GeometryHeight", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.height = value.value_or(b.geometry.height);
      return value.has_value();
    }
  },
  { "GeometryIsSmall", [](Objects::ObjectBuilder& b, INIFile& f) { b.geometry.small = f.parseBool(); return true; } },
  { "InstanceScaleFuzziness", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.scaleFuzziness = value.value_or(b.scaleFuzziness);
      return value.has_value();
    }
  },
  { "IsBridge", [](Objects::ObjectBuilder& b, INIFile& f) { b.isBridge = f.parseBool(); return true; } },
  { "IsTrainable", [](Objects::ObjectBuilder& b, INIFile& f) { b.trainable = f.parseBool(); return true; } },
  { "KindOf", [](Objects::ObjectBuilder& b, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(b.attributes, CALL(Objects::getAttribute));
    }
  },
  { "Locomotor", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto locomotors = parseLocomotors(f);
      if (locomotors.empty()) {
        return false;
      }

      b.locomotors.insert(b.locomotors.end(), locomotors.cbegin(), locomotors.cend());

      return true;
    }
  },
  { "MaxSimultaneousLinkKey", [](Objects::ObjectBuilder& b, INIFile& f) {
      b.simultaneousLimitRestrictionByKey = f.parseString();
      return !b.simultaneousLimitRestrictionByKey.empty();
    }
  },
  { "MaxSimultaneousOfType", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseString();
      if (value.empty()) {
        return false;
      }

      if (value == "DeterminedBySuperweaponRestriction") {
        b.simultaneousLimitByRestriction = true;
        return true;
      }

      auto opt = f.parseShort(value);
      if (!opt) {
        return false;
      }

      b.simultaneousLimit = opt;

      return true;
    }
  },
  { "PlacementViewAngle", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.placementAngle = value.value_or(b.placementAngle);
      return value.has_value();
    }
  },
  { "Prerequisites", [](Objects::ObjectBuilder& b, INIFile& f) { return f.parseAttributeBlock(b, PrerequisitesKVMap); } },
  { "RadarPriority", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto string = f.parseString();
      if (string.empty()) {
        return false;
      }

      auto opt = Objects::getRadarPriority(string);
      b.radarPriority = opt.value_or(b.radarPriority);

      return opt.has_value();
    }
  },
  { "RefundValue", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseShort();
      b.refundValue = value.value_or(b.refundValue);
      return value.has_value();
    }
  },
  { "Scale", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.scale = value.value_or(b.scale);
      return value.has_value();
    }
  },
  { "SelectPortrait", [](Objects::ObjectBuilder& b, INIFile& f) { b.portrait = f.parseString(); return !b.portrait.empty(); } },
  { "Shadow", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseEnum<Objects::Shadow>(CALL(Objects::getShadow));
      b.shadow.type = opt.value_or(Objects::Shadow::NONE);
      return opt.has_value();
    }
  },
  { "ShadowSizeX", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.shadow.size.x = value.value_or(0.0f);
      return value.has_value();
    }
  },
  { "ShadowSizeY", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.shadow.size.y = value.value_or(0.0f);
      return value.has_value();
    }
  },
  { "ShadowTexture", [](Objects::ObjectBuilder& b, INIFile& f) {
      b.shadow.texture = f.parseString();
      return !b.shadow.texture.empty();
    }
  },
  { "ShroudClearingRange", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.decloakingRange = value.value_or(b.decloakingRange);
      return value.has_value();
    }
  },
  { "ShroudRevealToAllRange", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.decloakedRange = value.value_or(b.decloakedRange);
      return value.has_value();
    }
  },
  { "Side", [](Objects::ObjectBuilder& b, INIFile& f) { b.side = f.parseString(); return !b.side.empty(); } },
  { "SoundAmbient", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_AMBIENT); } },
  { "SoundAmbientDamaged", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_AMBIENT_DAMAGED); } },
  { "SoundAmbientReallyDamaged", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_AMBIENT_REALLY_DAMAGED); } },
  { "SoundAmbientRubble", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_AMBIENT_RUBBLE); } },
  { "SoundEnter", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_ENTER); } },
  { "SoundExit", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_EXIT); } },
  { "SoundMoveLoop", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_MOVE_LOOP); } },
  { "SoundMoveStart", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_MOVE_START); } },
  { "SoundMoveStartDamaged", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_MOVE_START_DAMAGED); } },
  { "SoundOnDamaged", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_ON_DAMAGED); } },
  { "SoundOnReallyDamaged", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_ON_REALLY_DAMAGED); } },
  { "SoundStealthOn", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_STEALTH_ON); } },
  { "SoundStealthOff", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::SOUND_STEALTH_OFF); } },
  { "TransportSlotCount", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseInteger();
      if (!opt) {
        return false;
      }

      if (*opt > 0) {
        b.transportSlotCount = opt;
      }
      return opt.has_value();
    }
  },
  // Garbage workaround
  { "Upgrade_AmericaHallfireDrone", [](Objects::ObjectBuilder& b, INIFile& f) { return true; } },
  { "UpgradeCameo1", [](Objects::ObjectBuilder& b, INIFile& f) { b.upgradeCameos[0] = f.parseString(); return !b.upgradeCameos[0].empty(); } },
  { "UpgradeCameo2", [](Objects::ObjectBuilder& b, INIFile& f) { b.upgradeCameos[1] = f.parseString(); return !b.upgradeCameos[0].empty(); } },
  { "UpgradeCameo3", [](Objects::ObjectBuilder& b, INIFile& f) { b.upgradeCameos[2] = f.parseString(); return !b.upgradeCameos[0].empty(); } },
  { "UpgradeCameo4", [](Objects::ObjectBuilder& b, INIFile& f) { b.upgradeCameos[3] = f.parseString(); return !b.upgradeCameos[0].empty(); } },
  { "UpgradeCameo5", [](Objects::ObjectBuilder& b, INIFile& f) { b.upgradeCameos[4] = f.parseString(); return !b.upgradeCameos[0].empty(); } },
  { "UnitSpecificFX", [](Objects::ObjectBuilder& b, INIFile& f) { return f.parseAttributeBlock(b, UnitSpecificFXKV); } },
  { "UnitSpecificSounds", [](Objects::ObjectBuilder& b, INIFile& f) { return f.parseAttributeBlock(b, UnitSpecificSoundsKV); } },
  { "VisionRange", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.visualRange = value.value_or(b.visualRange);
      return value.has_value();
    }
  },
  { "VoiceAttack", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_ATTACK); } },
  { "VoiceAttackAir", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_ATTACK_AIR); } },
  // redundant use
  { "VoiceEnter", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_ENTER); } },
  { "VoiceFear", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_FEAR); } },
  { "VoiceGarrison", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_GARRISON); } },
  { "VoiceGuard", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_GUARD); } },
  { "VoiceMove", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_MOVE); } },
  { "VoiceSelect", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_SELECT); } },
  { "VoiceTaskComplete", [](Objects::ObjectBuilder& b, INIFile& f) { return parseSound(b, f, Objects::Noise::VOICE_TASK_COMPLETE); } },
  { "WeaponSet", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto& weaponSet = b.weaponSets.emplace_back();
      return f.parseAttributeBlock(weaponSet, WeaponSetKVMap);
    }
  },
};

static INIApplierMap<Objects::QueueProductionExit> QueueProductionExitKVMap = {
  { "ExitDelay", [](Objects::QueueProductionExit& qpe, INIFile& f) {
      auto opt = f.parseInteger();
      qpe.exitDelayMs = opt.value_or(qpe.exitDelayMs);
      return opt.has_value();
    }
  },
  { "NaturalRallyPoint", [](Objects::QueueProductionExit& qpe, INIFile& f) {
      auto coords = f.parseCoord3D();
      qpe.rallyPoint.x = coords[0];
      qpe.rallyPoint.y = coords[2];
      qpe.rallyPoint.z = coords[1];

      return true;
    }
  },
  { "UnitCreatePoint", [](Objects::QueueProductionExit& qpe, INIFile& f) {
      auto coords = f.parseCoord3D();
      qpe.createPoint.x = coords[0];
      qpe.createPoint.y = coords[2];
      qpe.createPoint.z = coords[1];

      return true;
    }
  }
};

static INIApplierMap<Objects::Radar> RadarKVMap = {
  { "RadarExtendTime", [](Objects::Radar& r, INIFile& f) {
      auto opt = f.parseInteger();
      r.extendTimeMs = opt.value_or(r.extendTimeMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::RailedTransportAI> RailedTransportAIKVMap = {
  { "PathPrefixName", [](Objects::RailedTransportAI& ai, INIFile& f) { ai.pathPrefix = f.parseString(); return !ai.pathPrefix.empty(); } },
};

static INIApplierMap<Objects::RailedTransportDock> RailedTransportDockKVMap = {
  { "PullInsideDuration", [](Objects::RailedTransportDock& rtd, INIFile& f) {
      auto opt = f.parseInteger();
      rtd.pullInsideDurationMs = opt.value_or(rtd.pullInsideDurationMs);
      return opt.has_value();
    }
  },
  { "PushOutsideDuration", [](Objects::RailedTransportDock& rtd, INIFile& f) {
      auto opt = f.parseInteger();
      rtd.pushOutsideDurationMs = opt.value_or(rtd.pushOutsideDurationMs);
      return opt.has_value();
    }
  },
  { "ToleranceDistance", [](Objects::RailedTransportDock& rtd, INIFile& f) {
      auto opt = f.parseInteger();
      rtd.toleranceDist = opt.value_or(rtd.toleranceDist);
      return opt.has_value();
    }
  }
};

// TODO
static INIApplierMap<Objects::RailroadBehavior> RailroadBehaviorKVMap = {
  { "*", SKIP(Objects::RailroadBehavior) }
};

static INIApplierMap<Objects::RebuildHoleBehavior> RebuildHoleBehaviorKVMap = {
  { "HoleHealthRegen%PerSecond", [](Objects::RebuildHoleBehavior& rh, INIFile& f) {
      auto opt = f.parsePercent();
      rh.healthRegenPerSecondPercent = opt.value_or(rh.healthRegenPerSecondPercent);
      return opt.has_value();
    }
  },
  { "WorkerObjectName", [](Objects::RebuildHoleBehavior& rh, INIFile& f) { rh.worker = f.parseString(); return !rh.worker.empty(); } },
  { "WorkerRespawnDelay", [](Objects::RebuildHoleBehavior& rh, INIFile& f) {
      auto opt = f.parseSignedInteger();
      rh.workerRespawnDelayMs = opt.value_or(rh.workerRespawnDelayMs);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::RebuildHoleExposeDie> RebuildHoleExposeDieKVMap = {
  { "HoleMaxHealth", [](Objects::RebuildHoleExposeDie& rh, INIFile& f) {
      auto opt = f.parseFloat();
      rh.maxHealth = opt.value_or(rh.maxHealth);
      return opt.has_value();
    }
  },
  { "HoleName", [](Objects::RebuildHoleExposeDie& rh, INIFile& f) { rh.name = f.parseString(); return !rh.name.empty(); } },
  { "TransferAttackers", [](Objects::RebuildHoleExposeDie& rh, INIFile& f) { rh.transferAttackers = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::Spawn> SpawnKVMap = {
  { "CanReclaimOrphans", [](Objects::Spawn& s, INIFile& f) { s.reclaimOrphans = f.parseBool(); return true; } },
  { "OneShot", [](Objects::Spawn& s, INIFile& f) { s.once = f.parseBool(); return true; } },
  { "SlavesHaveFreeWill", [](Objects::Spawn& s, INIFile& f) { s.spawnsWithFreeWill = f.parseBool(); return true; } },
  { "SpawnedRequireSpawner", [](Objects::Spawn& s, INIFile& f) { s.requiresSpawner = f.parseBool(); return true; } },
  { "SpawnNumber", [](Objects::Spawn& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.number = opt.value_or(s.number);
      return opt.has_value();
    }
  },
  { "SpawnReplaceDelay", [](Objects::Spawn& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.replaceDelayMs = opt.value_or(s.replaceDelayMs);
      return opt.has_value();
    }
  },
  { "SpawnTemplateName", [](Objects::Spawn& s, INIFile& f) { s.spawn = f.parseString(); return !s.spawn.empty(); } }
};

static INIApplierMap<Objects::SpawnPointProductionExit> SpawnPointProductionExitKVMap = {
  { "SpawnPointBoneName", [](Objects::SpawnPointProductionExit& se, INIFile& f) { se.spawnBone = f.parseString(); return !se.spawnBone.empty(); } }
};

static INIApplierMap<Objects::Slaved> SlavedKVMap = {
  { "AttackRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.attackRange = opt.value_or(s.attackRange);
      return opt.has_value();
    }
  },
  { "AttackWanderRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.attackWanderRange = opt.value_or(s.attackWanderRange);
      return opt.has_value();
    }
  },
  { "DistToTargetToGrantRangeBonus", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.targetMasterBonusRange = opt.value_or(s.targetMasterBonusRange);
      return opt.has_value();
    }
  },
  { "GuardMaxRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.guardMaxRange = opt.value_or(s.guardMaxRange);
      return opt.has_value();
    }
  },
  { "GuardWanderRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.guardWanderRange = opt.value_or(s.guardWanderRange);
      return opt.has_value();
    }
  },
  { "RepairMaxAltitude", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseFloat();
      s.repairMaxAlt = opt.value_or(s.repairMaxAlt);
      return opt.has_value();
    }
  },
  { "RepairMaxReadyTime", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.readyMaxMs = opt.value_or(s.readyMaxMs);
      return opt.has_value();
    }
  },
  { "RepairMaxWeldTime", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.weldMaxMs = opt.value_or(s.weldMaxMs);
      return opt.has_value();
    }
  },
  { "RepairMinAltitude", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseFloat();
      s.repairMinAlt = opt.value_or(s.repairMinAlt);
      return opt.has_value();
    }
  },
  { "RepairMinReadyTime", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.readyMinMs = opt.value_or(s.readyMinMs);
      return opt.has_value();
    }
  },
  { "RepairMinWeldTime", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.weldMinMs = opt.value_or(s.weldMinMs);
      return opt.has_value();
    }
  },
  { "RepairRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.repairRange = opt.value_or(s.repairRange);
      return opt.has_value();
    }
  },
  { "RepairRatePerSecond", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseFloat();
      s.repairRate = opt.value_or(s.repairRate);
      return opt.has_value();
    }
  },
  { "RepairWeldingFXBone", [](Objects::Slaved& s, INIFile& f) { s.weldingEffectBone = f.parseString(); return !s.weldingEffectBone.empty(); } },
  { "RepairWeldingSys", [](Objects::Slaved& s, INIFile& f) { s.welding = f.parseString(); return !s.welding.empty(); } },
  { "RepairWhenBelowHealth%", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parsePercent();
      s.repairWhenHealthBelow = opt.value_or(s.repairWhenHealthBelow);
      return opt.has_value();
    }
  },
  { "ScoutRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.scoutRange = opt.value_or(s.scoutRange);
      return opt.has_value();
    }
  },
  { "ScoutWanderRange", [](Objects::Slaved& s, INIFile& f) {
      auto opt = f.parseSignedInteger();
      s.scoutWanderRange = opt.value_or(s.scoutWanderRange);
      return opt.has_value();
    }
  },
  { "StayOnSameLayerAsMaster", [](Objects::Slaved& s, INIFile& f) { s.sameLayer = f.parseBool(); return true; } }
};

static std::optional<Objects::SlowDeathCreationList> parseSlowDeathCreationList(INIFile& f) {
  auto value = f.parseStringList();
  if (value.size() != 2) {
    return {};
  }

  auto opt = Objects::getSlowDeathPhase(value[0]);
  if (!opt) {
    return {};
  }

  Objects::SlowDeathCreationList sde;
  sde.creationList = value[1];
  sde.phase = *opt;

  return {std::move(sde)};
};

static std::optional<Objects::SlowDeathEffect> parseSlowDeathEffect(INIFile& f) {
  auto value = f.parseStringList();
  if (value.size() != 2) {
    return {};
  }

  auto opt = Objects::getSlowDeathPhase(value[0]);
  if (!opt) {
    return {};
  }

  Objects::SlowDeathEffect sde;
  sde.effect = value[1];
  sde.phase = *opt;

  return {std::move(sde)};
};

static INIApplierMap<Objects::SlowDeath> SlowDeathKVMap = {
  { "DestructionDelay", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = f.parseInteger();
      sd.destructionDelayMs = opt.value_or(sd.destructionDelayMs);
      return opt.has_value();
    }
  },
  { "DestructionDelayVariance", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = f.parseInteger();
      sd.destructionDelayVarianceMs = opt.value_or(sd.destructionDelayVarianceMs);
      return opt.has_value();
    }
  },
  { "DeathTypes", [](Objects::SlowDeath& sd, INIFile& f) {
      return f.parseEnumSet<Objects::DeathType>(sd.deathTypes, CALL(Objects::getDeathType));
    }
  },
  { "FX", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = parseSlowDeathEffect(f);
      if (opt) {
        sd.effects.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "FlingForce", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.flingForce = value.value_or(sd.flingForce);
      return value.has_value();
    }
  },
  { "FlingForceVariance", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.flingForceVariance = value.value_or(sd.flingForceVariance);
      return value.has_value();
    }
  },
  { "FlingPitch", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.flingPitch = value.value_or(sd.flingPitch);
      return value.has_value();
    }
  },
  { "FlingPitchVariance", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.flingPitchVariance = value.value_or(sd.flingPitchVariance);
      return value.has_value();
    }
  },
  { "OCL", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = parseSlowDeathCreationList(f);
      if (opt) {
        sd.creationLists.push_back(*opt);
      }
      return opt.has_value();
    }
  },
  { "ProbabilityModifier", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = f.parseSignedInteger();
      sd.probability = opt.value_or(sd.probability);
      return opt.has_value();
    }
  },
  { "SinkRate", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.sinkRate = value.value_or(sd.sinkRate);
      return value.has_value();
    }
  },
  { "SinkDelay", [](Objects::SlowDeath& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.sinkDelayMs = opt.value_or(fd.sinkDelayMs);
      return opt.has_value();
    }
  },
  { "SinkDelayVariance", [](Objects::SlowDeath& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.sinkDelayVarianceMs = opt.value_or(fd.sinkDelayVarianceMs);
      return opt.has_value();
    }
  },
  { "Weapon", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = parseSlowDeathEffect(f);
      if (opt) {
        sd.weapons.push_back(*opt);
      }
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::SpecialPower> SpecialPowerKVMap = {
  { "InitiateSound", [](Objects::SpecialPower& sp, INIFile& f) { sp.sound = f.parseString(); return !sp.sound.empty(); } },
  { "ScriptedSpecialPowerOnly", [](Objects::SpecialPower& sp, INIFile& f) { sp.scriptedOnly = f.parseBool(); return true; } },
  { "SpecialPowerTemplate", [](Objects::SpecialPower& sd, INIFile& f) { sd.specialPower = f.parseString(); return !sd.specialPower.empty(); } },
  { "StartsPaused", [](Objects::SpecialPower& sd, INIFile& f) { sd.paused = f.parseBool(); return true; } },
  { "UpdateModuleStartsAttack", [](Objects::SpecialPower& sd, INIFile& f) { sd.updateStartsAttack = f.parseBool(); return true; } }
};

static INIApplierMap<Objects::SpecialPowerUpdate> SpecialPowerUpdateKVMap = {
  { "AbilityAbortRange", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.abilityAbortRange = value.value_or(sd.abilityAbortRange);
      return value.has_value();
    }
  },
  { "AlwaysValidateSpecialObjects", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.validateSpecialObject = f.parseBool(); return true; } },
  { "AwardXPForTriggering", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.xpAward = value.value_or(sd.xpAward);
      return value.has_value();
    }
  },
  { "DoCaptureFX", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.captureEffect = f.parseString(); return !sd.captureEffect.empty(); } },
  { "FleeRangeAfterCompletion", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.fleeRange = value.value_or(sd.fleeRange);
      return value.has_value();
    }
  },
  { "FlipOwnerAfterUnpacking", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.switchOwnerAfterUnpacking = f.parseBool(); return true; } },
  { "LoseStealthOnTrigger", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.loseStealth = f.parseBool(); return true; } },
  { "MaxSpecialObjects", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseShort();
      sd.maxSpecialObjects = value.value_or(sd.maxSpecialObjects);
      return value.has_value();
    }
  },
  { "PackTime", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.packTimeMs = value.value_or(sd.packTimeMs);
      return value.has_value();
    }
  },
  { "PersistentPrepTime", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.persistentPreparationTimeMs = value.value_or(sd.persistentPreparationTimeMs);
      return value.has_value();
    }
  },
  { "PreparationTime", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.preparationTimeMs = value.value_or(sd.preparationTimeMs);
      return value.has_value();
    }
  },
  { "PreTriggerUnstealthTime", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.unstealthTimeMs = value.value_or(sd.unstealthTimeMs);
      return value.has_value();
    }
  },
  { "SkipPackingWithNoTarget", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.skipPackingWithoutTarget = f.parseBool(); return true; } },
  { "SpecialObject", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialObject = f.parseString(); return !sd.specialObject.empty(); } },
  { "SpecialObjectAttachToBone", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialObjectToBone = f.parseString(); return !sd.specialObjectToBone.empty(); } },
  { "SpecialObjectsPersistent", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialObjectPersists = f.parseBool(); return true; } },
  { "SpecialObjectsPersistWhenOwnerDies", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialObjectPersistsOnDeath = f.parseBool(); return true; } },
  { "SpecialPowerTemplate", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialPower = f.parseString(); return !sd.specialPower.empty(); } },
  { "StartAbilityRange", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.abilityStartRange = value.value_or(sd.abilityStartRange);
      return value.has_value();
    }
  },
  { "UniqueSpecialObjectTargets", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.specialObjectUniquePerTarget = f.parseBool(); return true; } },
  { "UnpackSound", [](Objects::SpecialPowerUpdate& sd, INIFile& f) { sd.unpackSound = f.parseString(); return !sd.unpackSound.empty(); } },
  { "UnpackTime", [](Objects::SpecialPowerUpdate& sd, INIFile& f) {
      auto value = f.parseInteger();
      sd.unpackTimeMs = value.value_or(sd.unpackTimeMs);
      return value.has_value();
    }
  },
};

// TODO
static INIApplierMap<Objects::SpectreGunship> SpectreGunshipKVMap = {
  { "AttackAreaDecal", [](Objects::SpectreGunship& sd, INIFile& f) {
      return f.parseAttributeBlock(sd, SpectreGunshipKVMap);
    }
  },
  { "TargetingReticleDecal", [](Objects::SpectreGunship& sd, INIFile& f) {
      return f.parseAttributeBlock(sd, SpectreGunshipKVMap);
    }
  },
  { "*", SKIP(Objects::SpectreGunship) }
};

// TODO
static INIApplierMap<Objects::SpectreGunshipDeployment> SpectreGunshipDeploymentKVMap = {
  { "*", SKIP(Objects::SpectreGunshipDeployment) }
};

static INIApplierMap<Objects::Stealth> StealthKVMap = {
  { "EnemyDetectionEvaEvent", [](Objects::Stealth& s, INIFile& f) { s.enemyDetectionEvaEvent = f.parseString(); return !s.enemyDetectionEvaEvent.empty(); } },
  { "FriendlyOpacityMax", [](Objects::Stealth& s, INIFile& f) {
      auto opt = f.parsePercent();
      s.friendlyOpacityMax = opt.value_or(s.friendlyOpacityMax);
      return opt.has_value();
    }
  },
  { "FriendlyOpacityMin", [](Objects::Stealth& s, INIFile& f) {
      auto opt = f.parsePercent();
      s.friendlyOpacityMin = opt.value_or(s.friendlyOpacityMin);
      return opt.has_value();
    }
  },
  { "HintDetectableConditions", [](Objects::Stealth& s, INIFile& f) {
      return f.parseEnumSet<Objects::Status>(s.detectableStates, CALL(Objects::getStatus));
    }
  },
  { "InnateStealth", [](Objects::Stealth& s, INIFile& f) { s.innateStealth = f.parseBool(); return true; } },
  { "MoveThresholdSpeed", [](Objects::Stealth& s, INIFile& f) {
      auto value = f.parseFloat();
      s.moveSpeedThreshold = value.value_or(s.moveSpeedThreshold);
      return value.has_value();
    }
  },
  { "OrderIdleEnemiesToAttackMeUponReveal", [](Objects::Stealth& s, INIFile& f) { s.gettingAttackWhenRevealed = f.parseBool(); return true; } },
  { "OwnDetectionEvaEvent", [](Objects::Stealth& s, INIFile& f) { s.ownDetectionEvaEvent = f.parseString(); return !s.ownDetectionEvaEvent.empty(); } },
  { "PulseFrequency", [](Objects::Stealth& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.pulseFrequencyMs = opt.value_or(s.pulseFrequencyMs);
      return opt.has_value();
    }
  },
  { "StealthDelay", [](Objects::Stealth& s, INIFile& f) {
      auto opt = f.parseInteger();
      s.delayMs = opt.value_or(s.delayMs);
      return opt.has_value();
    }
  },
  { "StealthForbiddenConditions", [](Objects::Stealth& s, INIFile& f) {
      return f.parseEnumSet<Objects::StealthLevel>(s.forbiddenConditions, CALL(Objects::getStealthLevel));
    }
  }
};

static INIApplierMap<Objects::SpyVision> SpyVisionKVMap = {
  { "NeedsUpgrade", [](Objects::SpyVision& sv, INIFile& f) { sv.needsUpgrade = f.parseBool(); return true; } },
  { "SelfPowered", [](Objects::SpyVision& sv, INIFile& f) { sv.selfPowered = f.parseBool(); return true; } },
  { "SelfPoweredDuration", [](Objects::SpyVision& sv, INIFile& f) {
      auto opt = f.parseInteger();
      sv.selfPoweredDurationMs = opt.value_or(sv.selfPoweredDurationMs);
      return opt.has_value();
    }
  },
  { "SelfPoweredInterval", [](Objects::SpyVision& sv, INIFile& f) {
      auto opt = f.parseInteger();
      sv.selfPoweredIntervalMs = opt.value_or(sv.selfPoweredIntervalMs);
      return opt.has_value();
    }
  },
  { "SpyOnKindof", [](Objects::SpyVision& sv, INIFile& f) { return f.parseEnumSet<Objects::Attribute>(sv.spyOn, CALL(Objects::getAttribute)); } }
};

static INIApplierMap<Objects::SpyVisionSpecialPower> SpyVisionSpecialPowerKVMap = {
  { "BaseDuration", [](Objects::SpyVisionSpecialPower& svp, INIFile& f) {
      auto opt = f.parseInteger();
      svp.baseDurationMs = opt.value_or(svp.baseDurationMs);
      return opt.has_value();
    }
  },
  { "BonusDurationPerCaptured", [](Objects::SpyVisionSpecialPower& svp, INIFile& f) {
      auto opt = f.parseInteger();
      svp.bonusDurationPerCaptureMs = opt.value_or(svp.bonusDurationPerCaptureMs);
      return opt.has_value();
    }
  },
  { "MaxDuration", [](Objects::SpyVisionSpecialPower& svp, INIFile& f) {
      auto opt = f.parseInteger();
      svp.maxDurationMs = opt.value_or(svp.maxDurationMs);
      return opt.has_value();
    }
  }
};


static INIApplierMap<Objects::StealthDetector> StealthDetectorKVMap = {
  { "CanDetectWhileContained", [](Objects::StealthDetector& sd, INIFile& f) { sd.detectWhenContained = f.parseBool(); return true; } },
  { "CanDetectWhileGarrisoned", [](Objects::StealthDetector& sd, INIFile& f) { sd.detectWhenGarrisoned = f.parseBool(); return true; } },
  { "DetectionRange", [](Objects::StealthDetector& sd, INIFile& f) {
      auto opt = f.parseFloat();
      sd.range = opt.value_or(sd.range);
      return opt.has_value();
    }
  },
  { "DetectionRate", [](Objects::StealthDetector& sd, INIFile& f) {
      auto opt = f.parseInteger();
      sd.rateMs = opt.value_or(sd.rateMs);
      return opt.has_value();
    }
  },
  { "ExtraForbiddenKindOf", [](Objects::StealthDetector& sd, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(sd.detectionExclusion, CALL(Objects::getAttribute));
    }
  },
  { "ExtraRequiredKindOf", [](Objects::StealthDetector& sd, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(sd.detectionInclusion, CALL(Objects::getAttribute));
    }
  },
  { "InitiallyDisabled", [](Objects::StealthDetector& sd, INIFile& f) { sd.disabled = f.parseBool(); return true; } },
  { "IRBeaconParticleSysName", [](Objects::StealthDetector& sd, INIFile& f) { sd.beaconParticles = f.parseString(); return !sd.beaconParticles.empty(); } },
  { "IRParticleSysBone", [](Objects::StealthDetector& sd, INIFile& f) { sd.particlesBone = f.parseString(); return !sd.particlesBone.empty(); } },
  { "LoudPingSound", [](Objects::StealthDetector& sd, INIFile& f) { sd.loudPingSound = f.parseString(); return !sd.loudPingSound.empty(); } },
  { "PingSound", [](Objects::StealthDetector& sd, INIFile& f) { sd.pingSound = f.parseString(); return !sd.pingSound.empty(); } }
};

std::optional<Objects::CollapseEvent> parseStructureCollapse(INIFile& f) {
  auto values = f.parseStringList();
  if (values.size() != 2) {
    return {};
  }

  Objects::CollapseEvent event;
  auto opt = Objects::getStructureCollapsePhase(values[0]);
  if (!opt) {
    return {};
  }
  event.phase = *opt;
  event.event = std::move(values[1]);

  return {std::move(event)};
};

static INIApplierMap<Objects::StructureCollapse> StructureCollapseKVMap = {
  { "BigBurstFrequency", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseSignedInteger();
      sc.bigBurstFrequency = opt.value_or(sc.bigBurstFrequency);
      return opt.has_value();
    }
  },
  { "CollapseDamping", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseFloat();
      sc.collapseDampening = opt.value_or(sc.collapseDampening);
      return opt.has_value();
    }
  },
  { "FXList", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = parseStructureCollapse(f);
      if (!opt) {
        return false;
      }

      sc.effects.emplace_back(std::move(*opt));
      return true;
    }
  },
  { "MaxBurstDelay", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseInteger();
      sc.maxBurstDelayMs = opt.value_or(sc.maxBurstDelayMs);
      return opt.has_value();
    }
  },
  { "MaxCollapseDelay", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseInteger();
      sc.maxCollapseDelayMs = opt.value_or(sc.maxCollapseDelayMs);
      return opt.has_value();
    }
  },
  { "MaxShudder", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseFloat();
      sc.maxShudder = opt.value_or(sc.maxShudder);
      return opt.has_value();
    }
  },
  { "MinBurstDelay", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseInteger();
      sc.minBurstDelayMs = opt.value_or(sc.minBurstDelayMs);
      return opt.has_value();
    }
  },
  { "MinCollapseDelay", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = f.parseInteger();
      sc.minCollapseDelayMs = opt.value_or(sc.minCollapseDelayMs);
      return opt.has_value();
    }
  },
  { "OCL", [](Objects::StructureCollapse& sc, INIFile& f) {
      auto opt = parseStructureCollapse(f);
      if (!opt) {
        return false;
      }

      sc.creationLists.emplace_back(std::move(*opt));
      return true;
    }
  }
};

static INIApplierMap<Objects::StructureTopple> StructureToppleKVMap = {
  { "AngleFX", [](Objects::StructureTopple& st, INIFile& f) {
      auto values = f.parseStringList();
      if (values.size() != 2) {
        return false;
      }

      auto opt = f.parseFloat(values[0]);
      if (!opt) {
        return false;
      }
      st.effectAngle = *opt;
      st.angleEffectName = std::move(values[1]);

      return true;
    }
  },
  { "CrushingFX", [](Objects::StructureTopple& st, INIFile& f) { st.crushingEffect = f.parseString(); return !st.crushingEffect.empty(); } },
  { "CrushingWeaponName", [](Objects::StructureTopple& st, INIFile& f) { st.crushingWeapon = f.parseString(); return !st.crushingWeapon.empty(); } },
  { "DamageFXTypes", [](Objects::StructureTopple& st, INIFile& f) {
      return f.parseEnumSet<Objects::DamageType>(st.damageTypes, CALL(Objects::getDamageType));
    }
  },
  { "MaxToppleBurstDelay", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseInteger();
      st.maxToppleBurstDelayMs = opt.value_or(st.maxToppleBurstDelayMs);
      return opt.has_value();
    }
  },
  { "MaxToppleDelay", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseInteger();
      st.maxToppleDelayMs = opt.value_or(st.maxToppleDelayMs);
      return opt.has_value();
    }
  },
  { "MinToppleBurstDelay", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseInteger();
      st.minToppleBurstDelayMs = opt.value_or(st.minToppleBurstDelayMs);
      return opt.has_value();
    }
  },
  { "MinToppleDelay", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseInteger();
      st.minToppleDelayMs = opt.value_or(st.minToppleDelayMs);
      return opt.has_value();
    }
  },
  { "StructuralDecay", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseFloat();
      st.decay = opt.value_or(st.decay);
      return opt.has_value();
    }
  },
  { "StructuralIntegrity", [](Objects::StructureTopple& st, INIFile& f) {
      auto opt = f.parseFloat();
      st.integrity = opt.value_or(st.integrity);
      return opt.has_value();
    }
  },
  { "ToppleDelayFX", [](Objects::StructureTopple& st, INIFile& f) { st.topplingDelayEffect = f.parseString(); return !st.topplingDelayEffect.empty(); } },
  { "ToppleDoneFX", [](Objects::StructureTopple& st, INIFile& f) { st.topplingDoneEffect = f.parseString(); return !st.topplingDoneEffect.empty(); } },
  { "ToppleStartFX", [](Objects::StructureTopple& st, INIFile& f) { st.topplingStartEffect = f.parseString(); return !st.topplingStartEffect.empty(); } },
};

static INIApplierMap<Objects::Topple> ToppleKVMap = {
  { "BounceFX", [](Objects::Topple& t, INIFile& f) { t.bounceEffect = f.parseString(); return !t.bounceEffect.empty(); } },
  { "BounceVelocityPercent", [](Objects::Topple& t, INIFile& f) {
      auto opt = f.parsePercent();
      t.bounceVelocity = opt.value_or(t.bounceVelocity);
      return opt.has_value();
    }
  },
  { "InitialAccelPercent", [](Objects::Topple& t, INIFile& f) {
      auto opt = f.parsePercent();
      t.initialAcceleration = opt.value_or(t.initialAcceleration);
      return opt.has_value();
    }
  },
  { "KillWhenStartToppling", [](Objects::Topple& t, INIFile& f) { t.killOnStartToppling = f.parseBool(); return true; } },
  { "ReorientToppledRubble", [](Objects::Topple& t, INIFile& f) { t.reorientToppledRubble = f.parseBool(); return true; } },
  { "StumpName", [](Objects::Topple& t, INIFile& f) { t.stump = f.parseString(); return !t.stump.empty(); } },
  { "ToppleFX", [](Objects::Topple& t, INIFile& f) { t.toppleEffect = f.parseString(); return !t.toppleEffect.empty(); } },
  { "ToppleLeftOrRightOnly", [](Objects::Topple& t, INIFile& f) { t.oneAxisOnly = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::TreeDrawData> TreeDrawDataKVMap = {
  { "BounceFX", [](Objects::TreeDrawData& t, INIFile& f) { t.bounceEffect = f.parseString(); return !t.bounceEffect.empty(); } },
  { "DoShadow", [](Objects::TreeDrawData& t, INIFile& f) { t.shadow = f.parseBool(); return true; } },
  { "DoTopple", [](Objects::TreeDrawData& t, INIFile& f) { t.topple = f.parseBool(); return true; } },
  { "KillWhenFinishedToppling", [](Objects::TreeDrawData& t, INIFile& f) { t.killOnStopToppling = f.parseBool(); return true; } },
  { "ModelName", [](Objects::TreeDrawData& t, INIFile& f) { t.model = f.parseString(); return !t.model.empty(); } },
  { "SinkDistance", [](Objects::TreeDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.sinkDistance = value.value_or(t.sinkDistance);
      return value.has_value();
    }
  },
  { "SinkTime", [](Objects::TreeDrawData& t, INIFile& f) {
      auto opt = f.parseInteger();
      t.sinkTimeMs = opt.value_or(t.sinkTimeMs);
      return opt.has_value();
    }
  },
  { "StumpName", [](Objects::TreeDrawData& t, INIFile& f) { t.stump = f.parseString(); return !t.stump.empty(); } },
  { "TextureName", [](Objects::TreeDrawData& t, INIFile& f) { t.texture = f.parseString(); return !t.texture.empty(); } },
  { "ToppleFX", [](Objects::TreeDrawData& t, INIFile& f) { t.toppleEffect = f.parseString(); return !t.toppleEffect.empty(); } },
};

static bool parseTireBone(INIFile& f, std::array<std::string, 10>& tireBones, size_t idx) {
  tireBones[idx] = f.parseString();
  return !tireBones[idx].empty();
};

static INIApplierMap<Objects::TankDrawData> TankDrawDataKVMap = {
  { "TreadAnimationRate", [](Objects::TankDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.treadAnimationRate = value.value_or(t.treadAnimationRate);
      return value.has_value();
    }
  },
  { "TreadDriveSpeedFraction", [](Objects::TankDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.treadDriveSpeedFraction = value.value_or(t.treadDriveSpeedFraction);
      return value.has_value();
    }
  },
  { "TreadPivotSpeedFraction", [](Objects::TankDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.treadPivotSpeedFraction = value.value_or(t.treadPivotSpeedFraction);
      return value.has_value();
    }
  },
  { "TreadDebrisLeft", [](Objects::TankDrawData& t, INIFile& f) { t.treadDebrisLeft = f.parseString(); return !t.treadDebrisLeft.empty(); } },
  { "TreadDebrisRight", [](Objects::TankDrawData& t, INIFile& f) { t.treadDebrisRight = f.parseString(); return !t.treadDebrisRight.empty(); } },
};

static INIApplierMap<Objects::TruckDrawData> TruckDrawDataKVMap = {
  { "CabBone", [](Objects::TruckDrawData& t, INIFile& f) { t.cabBone = f.parseString(); return !t.cabBone.empty(); } },
  { "CabRotationMultiplier", [](Objects::TruckDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.cabRotationFactor = value.value_or(t.cabRotationFactor);
      return value.has_value();
    }
  },
  { "DirtSpray", [](Objects::TruckDrawData& t, INIFile& f) { t.dirtEffect = f.parseString(); return !t.dirtEffect.empty(); } },
  { "Dust", [](Objects::TruckDrawData& t, INIFile& f) { t.dustEffect = f.parseString(); return !t.dustEffect.empty(); } },
  { "LeftFrontTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 0); } },
  { "LeftRearTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 2); } },
  { "MidLeftFrontTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 4); } },
  { "MidLeftMidTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 8); } },
  { "MidLeftRearTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 6); } },
  { "MidRightFrontTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 5); } },
  { "MidRightMidTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 9); } },
  { "MidRightRearTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 7); } },
  { "PowerslideRotationAddition", [](Objects::TruckDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.powerslideRotationAddition = value.value_or(t.powerslideRotationAddition);
      return value.has_value();
    }
  },
  { "PowerslideSpray", [](Objects::TruckDrawData& t, INIFile& f) { t.powerslideEffect = f.parseString(); return !t.powerslideEffect.empty(); } },
  { "RightFrontTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 1); } },
  { "RightRearTireBone", [](Objects::TruckDrawData& t, INIFile& f) { return parseTireBone(f, t.tireBones, 3); } },
  { "RotationDamping", [](Objects::TruckDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.rotationDampeningFactor = value.value_or(t.rotationDampeningFactor);
      return value.has_value();
    }
  },
  { "TireRotationMultiplier", [](Objects::TruckDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.rotationSpeedMul = value.value_or(t.rotationSpeedMul);
      return value.has_value();
    }
  },
  { "TrailerBone", [](Objects::TruckDrawData& t, INIFile& f) { t.trailerBone = f.parseString(); return !t.trailerBone.empty(); } },
  { "TrailerRotationMultiplier", [](Objects::TruckDrawData& t, INIFile& f) {
      auto value = f.parseFloat();
      t.trailerRotationFactor = value.value_or(t.trailerRotationFactor);
      return value.has_value();
    }
  },
};

static INIApplierMap<Objects::SupplyDrawData> SupplyDrawDataKVMap = {
  { "SupplyBonePrefix", [](Objects::SupplyDrawData& sd, INIFile& f) { sd.supplyBonePrefix = f.parseString(); return !sd.supplyBonePrefix.empty(); } },
};

static INIApplierMap<Objects::VeterancyCrateCollision> VeterancyCrateCollisionKVMap = {
  { "AddsOwnerVeterancy", [](Objects::VeterancyCrateCollision& vc, INIFile& f) { vc.veterancyToTarget = f.parseBool(); return true; } },
  { "EffectRange", [](Objects::VeterancyCrateCollision& vc, INIFile& f) {
      auto opt = f.parseInteger();
      vc.effectRange = opt.value_or(vc.effectRange);
      return opt.has_value();
    }
  },
  { "IsPilot", [](Objects::VeterancyCrateCollision& vc, INIFile& f) { vc.isPilot = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::VeterancyGain> VeterancyGainKVMap = {
  { "ScienceRequired", [](Objects::VeterancyGain& vg, INIFile& f) { vg.requiredScience = f.parseString(); return !vg.requiredScience.empty(); } },
  { "StartingLevel", [](Objects::VeterancyGain& vg, INIFile& f) {
      auto opt = Objects::getVeterancy(f.parseString());
      vg.starting = opt.value_or(vg.starting);
      return opt.has_value();
    }
  }
};

ObjectsINI::ObjectsINI(std::istream& stream) : INIFile(stream) {}

ObjectsINI::ObjectMap ObjectsINI::parse() {
  ObjectMap objects;

  while (!stream.eof()) {
    auto token = consumeComment();
    if (token != "ObjectReskin" && token != "Object") {
      return objects;
    }

    if (!parseObject(objects, token == "ObjectReskin")) {
      erroneousObject = true;
    }
  }

  return objects;
}

bool ObjectsINI::parseObject(ObjectMap& objects, bool reskinning) {
  advanceStream();
  auto key = getTokenInLine();

  Objects::ObjectBuilder builder;

  if (reskinning) {
    advanceStream();
    auto reskinFrom = getTokenInLine();

    MurmurHash3_32 hasher;
    hasher.feed(reskinFrom);

    auto lookup = objects.find(hasher.getHash());
    if (lookup != objects.cend()) {
      builder = *lookup->second;
      // EVAL
      builder.drawMetaData.clear();
    } else {
      WARN_ZH("ObjectsINI", "Cannot reskin from {}", reskinFrom);
    }
  }

  builder.name = std::move(key);

  if (parseAttributeBlock(builder, ObjectDataKVMap)) {
    MurmurHash3_32 hasher;
    hasher.feed(builder.name);

    objects.emplace(
        hasher.getHash()
      , std::make_shared<Objects::ObjectBuilder>(std::move(builder))
    );
    return true;
  }

  return false;
}

bool ObjectsINI::parseBehavior(Objects::ObjectBuilder& builder) {
  auto values = parseStringList();
  if (values.empty()) {
    return false;
  }

  auto behaviorType = Objects::getModuleType(values[0]);
  if (!behaviorType) {
    WARN_ZH("ObjectsINI", "Unsupported behavior type: {}", values[0]);
    return false;
  }

  auto& behavior = builder.behaviors.emplace_back();
  behavior.type = *behaviorType;

  if (values.size() > 1) {
    behavior.moduleTag = values[1];
  }
  // There may be garbage after the tag

  switch (behavior.type) {
    case Objects::ModuleType::BASE_REGENERATE:
    case Objects::ModuleType::BONE_FX_DAMAGE:
    case Objects::ModuleType::BRIDGE_TOWER:
    case Objects::ModuleType::CHECKPOINT:
    case Objects::ModuleType::COMMAND_BUTTON_HUNT:
    case Objects::ModuleType::DAM_DIE:
    case Objects::ModuleType::ENEMY_NEAR:
    case Objects::ModuleType::KEEP_OBJECT_DIE:
    case Objects::ModuleType::PREORDER_CREATE:
    case Objects::ModuleType::RADIUS_DECAL:
    case Objects::ModuleType::SPECIAL_POWER_CREATE:
    case Objects::ModuleType::SQUISH_COLLIDE:
    case Objects::ModuleType::SUPPLY_CENTER:
    case Objects::ModuleType::SUPPLY_WAREHOUSE:
    case Objects::ModuleType::TECH_BUILDING:
      return parseEmptyAttributeBlock();

    case Objects::ModuleType::AI:
    case Objects::ModuleType::TRANSPORT_AI:
      return parseSubtypedAttributeBlock<Objects::AI>(std::move(behavior.moduleData), AIKVMap);
    case Objects::ModuleType::ASSISTED_TARGETING:
      return parseSubtypedAttributeBlock<Objects::AssistedTargeting>(std::move(behavior.moduleData), AssistedTargetingKVMap);
    case Objects::ModuleType::ASSAULT_TRANSPORT:
      return
        parseSubtypedAttributeBlocks<Objects::AssaultTransport>(
            std::move(behavior.moduleData)
          , AssaultTransportKVMap
          , AIKVMap
        );
    case Objects::ModuleType::AUTO_DEPOSIT:
      return parseSubtypedAttributeBlock<Objects::AutoDeposit>(std::move(behavior.moduleData), AutoDepositKVMap);
    case Objects::ModuleType::AUTO_FIND_HEALING:
      return parseSubtypedAttributeBlock<Objects::AutoFindHealing>(std::move(behavior.moduleData), AutoFindHealingKVMap);
    case Objects::ModuleType::AUTO_HEAL:
      return parseSubtypedAttributeBlock<Objects::AutoHeal>(std::move(behavior.moduleData), AutoHealKVMap);
    case Objects::ModuleType::BAIKONUR_LAUNCH_POWER:
      return
        parseSubtypedAttributeBlocks<Objects::BaikonurLaunchPower>(
            std::move(behavior.moduleData)
          , BaikonurLaunchPowerKVMap
          , SpecialPowerKVMap
        );
    case Objects::ModuleType::BATTLE_PLAN:
      return parseSubtypedAttributeBlock<Objects::BattlePlan>(std::move(behavior.moduleData), BattlePlanKVMap);
    case Objects::ModuleType::BONE_FX:
      return parseSubtypedAttributeBlock<Objects::BoneFX>(std::move(behavior.moduleData), BoneFXKVMap);
    case Objects::ModuleType::BRIDGE:
      return parseSubtypedAttributeBlock<Objects::Bridge>(std::move(behavior.moduleData), BridgeKVMap);
    case Objects::ModuleType::CASH_BOUNTY:
      return
        parseSubtypedAttributeBlocks<Objects::CashBounty>(
            std::move(behavior.moduleData)
          , CashBountyKVMap
          , SpecialPowerKVMap
        );
    case Objects::ModuleType::CASH_HACK:
      return
        parseSubtypedAttributeBlocks<Objects::CashHack>(
            std::move(behavior.moduleData)
          , CashHackKVMap
          , SpecialPowerKVMap
        );
    case Objects::ModuleType::CHINOOK_AI:
      return parseSubtypedAttributeBlock<Objects::ChinookAI>(std::move(behavior.moduleData), ChinookAIKVMap);
    case Objects::ModuleType::CLEANUP_AREA:
      return parseSubtypedAttributeBlock<Objects::CleanupArea>(std::move(behavior.moduleData), CleanupAreaKVMap);
    case Objects::ModuleType::CLEANUP_HAZARD:
      return parseSubtypedAttributeBlock<Objects::CleanupHazard>(std::move(behavior.moduleData), CleanupHazardKVMap);
    case Objects::ModuleType::COMMAND_SET_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::CommandSetUpgrade>(
            std::move(behavior.moduleData)
          , CommandSetUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::COST_MODIFIER_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::CostModifierUpgrade>(
            std::move(behavior.moduleData)
          , CostModifierUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::COUNTERMEASURE:
      return parseSubtypedAttributeBlock<Objects::Countermeasure>(std::move(behavior.moduleData), CountermeasureKVMap);
    case Objects::ModuleType::CREATE_CRATE_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::CreateCrateDie>(
            std::move(behavior.moduleData)
          , CreateCrateDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::CREATE_OBJECT_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::CreateObjectDie>(
            std::move(behavior.moduleData)
          , CreateObjectDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::CRUSH_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::CrushDie>(
            std::move(behavior.moduleData)
          , CrushDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::DEFAULT_PRORDUCTION_EXIT:
    case Objects::ModuleType::SUPPLY_CENTER_PRODUCTION_EXIT:
      return parseSubtypedAttributeBlock<Objects::DefaultProductionExit>(std::move(behavior.moduleData), DefaultProductionExitKVMap);
    case Objects::ModuleType::DELETION:
      return parseSubtypedAttributeBlock<Objects::Deletion>(std::move(behavior.moduleData), DeletionKVMap);
    case Objects::ModuleType::DELIVER_PAYLOAD:
      return parseSubtypedAttributeBlock<Objects::DeliverPayload>(std::move(behavior.moduleData), DeliverPayloadKVMap);
    case Objects::ModuleType::DEMO_TRAP:
      return parseSubtypedAttributeBlock<Objects::DemoTrap>(std::move(behavior.moduleData), DemoTrapKVMap);
    case Objects::ModuleType::DEPLOY_STYLE_AI:
      return
        parseSubtypedAttributeBlocks<Objects::DeployStyleAI>(
            std::move(behavior.moduleData)
          , DeployStyleAIKVMap
          , AIKVMap
        );
    case Objects::ModuleType::DESTROY_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::DestroyDie>(
            std::move(behavior.moduleData)
          , DestroyDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::DOZER_AI:
      return
        parseSubtypedAttributeBlocks<Objects::DozerAI>(
            std::move(behavior.moduleData)
          , DozerAIKVMap
          , AIKVMap
        );
    case Objects::ModuleType::DYNAMIC_SHROUD_CLEARING_RANGE:
      return parseSubtypedAttributeBlock<Objects::DynamicShroudClearingRange>(std::move(behavior.moduleData), DynamicShroudClearingRangeKVMap);
    case Objects::ModuleType::EJECT_PILOT_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::EjectPilotDie>(
            std::move(behavior.moduleData)
          , EjectPilotDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::EXPERINCE_SCALAR_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::ExperienceScalarUpgrade>(
            std::move(behavior.moduleData)
          , ExperienceScalarUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::FIRE_SPREAD:
      return parseSubtypedAttributeBlock<Objects::FireSpread>(std::move(behavior.moduleData), FireSpreadKVMap);
    case Objects::ModuleType::FIRE_WEAPON:
      return parseSubtypedAttributeBlock<Objects::FireWeapon>(std::move(behavior.moduleData), FireWeaponKVMap);
    case Objects::ModuleType::FIRE_WEAPON_COLLISION:
      return parseSubtypedAttributeBlock<Objects::FireWeaponCollision>(std::move(behavior.moduleData), FireWeaponCollisionDataKVMap);
    case Objects::ModuleType::FIRE_WEAPON_WHEN_DAMAGED:
      return parseSubtypedAttributeBlock<Objects::FireWeaponWhenDamaged>(std::move(behavior.moduleData), FireWeaponWhenDamagedKVMap);
    case Objects::ModuleType::FIRE_WEAPON_WHEN_DEAD:
      return parseSubtypedAttributeBlocks<Objects::FireWeaponWhenDead>(
          std::move(behavior.moduleData)
        , FireWeaponWhenDeadKVMap
        , UpgradeKVMap<Objects::DieUpgrade>
        , DieKVMap
      );
    case Objects::ModuleType::FLAMMABLE:
      return parseSubtypedAttributeBlock<Objects::Flammable>(std::move(behavior.moduleData), FlammableDataKVMap);
    case Objects::ModuleType::FLIGHT_DECK:
      return parseSubtypedAttributeBlock<Objects::FlightDeck>(std::move(behavior.moduleData), FlightDeckKVMap);
    case Objects::ModuleType::FLOAT:
      return parseSubtypedAttributeBlock<Objects::Float>(std::move(behavior.moduleData), FloatKVMap);
    case Objects::ModuleType::FX_LIST_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::FXListDie>(
            std::move(behavior.moduleData)
          , FXListDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::GARRISON_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::GarrisonContain>(
            std::move(behavior.moduleData)
          , GarrisonContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::GENERATE_MINEFIELD:
      return parseSubtypedAttributeBlock<Objects::GenerateMinefield>(std::move(behavior.moduleData), GenerateMinefieldKVMap);
    case Objects::ModuleType::GRANT_SCIENCE_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::GrantScienceUpgrade>(
            std::move(behavior.moduleData)
          , GrantScienceUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::GRANT_UPGRADE:
      return parseSubtypedAttributeBlock<Objects::GrantUpgrade>(std::move(behavior.moduleData), GrantUpgradeKVMap);
    case Objects::ModuleType::HEAL_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::HealContain>(
            std::move(behavior.moduleData)
          , HealContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::HEIGHT_DIE:
      return parseSubtypedAttributeBlock<Objects::HeightDie>(std::move(behavior.moduleData), HeightDieKVMap);
    case Objects::ModuleType::HELICOPTER_SLOW_DEATH:
      return
        parseSubtypedAttributeBlocks<Objects::HelicopterSlowDeath>(
            std::move(behavior.moduleData)
          , HelicopterSlowDeathKVMap
          , SlowDeathKVMap
        );
    case Objects::ModuleType::HELIX_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::HelixContain>(
            std::move(behavior.moduleData)
          , HelixContainKVMap
          , TransportContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::HORDE:
      return parseSubtypedAttributeBlock<Objects::Horde>(std::move(behavior.moduleData), HordeKVMap);
    case Objects::ModuleType::INSTANT_DEATH:
      return
        parseSubtypedAttributeBlocks<Objects::InstantDeath>(
            std::move(behavior.moduleData)
          , InstantDeathKVMap
          , DieKVMap
        );
    case Objects::ModuleType::LIFETIME:
      return parseSubtypedAttributeBlock<Objects::Lifetime>(std::move(behavior.moduleData), LifetimeDataKVMap);
    case Objects::ModuleType::LOCK_WEAPON:
      return parseSubtypedAttributeBlock<Objects::LockWeapon>(std::move(behavior.moduleData), LockWeaponKVMap);
    case Objects::ModuleType::JET_AI:
      return parseSubtypedAttributeBlock<Objects::JetAI>(std::move(behavior.moduleData), JetAIKVMap);
    case Objects::ModuleType::JET_SLOW_DEATH:
      return
        parseSubtypedAttributeBlocks<Objects::JetSlowDeath>(
            std::move(behavior.moduleData)
          , JetSlowDeathKV
          , SlowDeathKVMap
        );
    case Objects::ModuleType::MAX_HEALTH_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::MaxHealthUpgrade>(
            std::move(behavior.moduleData)
          , MaxHealthUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::MISSILE_AI:
      return parseSubtypedAttributeBlock<Objects::MissileAI>(std::move(behavior.moduleData), MissileAIKVMap);
    case Objects::ModuleType::MISSILE_LAUNCHER_BUILDING:
      return parseSubtypedAttributeBlock<Objects::MissileLauncherBuilding>(std::move(behavior.moduleData), MissileLauncherBuildingKVMap);
    case Objects::ModuleType::MODEL_CONDITION_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::ModelConditionUpgrade>(
            std::move(behavior.moduleData)
          , ModelConditionUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::OBJECT_CREATION_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::ObjectCreationUpgrade>(
            std::move(behavior.moduleData)
          , ObjectCreationUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::OCL:
      return parseSubtypedAttributeBlock<Objects::OCL>(std::move(behavior.moduleData), OCLKVMap);
    case Objects::ModuleType::OCL_SPECIAL_POWER:
      return
        parseSubtypedAttributeBlocks<Objects::OCLSpecialPower>(
            std::move(behavior.moduleData)
          , OCLSpecialPowerKVMap
          , SpecialPowerKVMap
        );
    case Objects::ModuleType::OVERCHARGE:
      return parseSubtypedAttributeBlock<Objects::Overcharge>(std::move(behavior.moduleData), OverchargeKVMap);
    case Objects::ModuleType::OVERLORD_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::OverlordContain>(
            std::move(behavior.moduleData)
          , OverlordContainKVMap
          , TransportContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::PARACHUTE_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::ParachuteContain>(
            std::move(behavior.moduleData)
          , ParachuteContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::PARKING_PLACE:
      return parseSubtypedAttributeBlock<Objects::ParkingPlace>(std::move(behavior.moduleData), ParkingPlaceKVMap);
    case Objects::ModuleType::PARTICLE_UPLINK_CANNON:
      return parseSubtypedAttributeBlock<Objects::ParticleUplinkCannon>(std::move(behavior.moduleData), ParticleUplinkCannonKVMap);
    case Objects::ModuleType::PHYSICS:
      return parseSubtypedAttributeBlock<Objects::Physics>(std::move(behavior.moduleData), PhysicsDataKVMap);
    case Objects::ModuleType::PILOT_FIND_VEHICLE:
      return parseSubtypedAttributeBlock<Objects::PilotFindVehicle>(std::move(behavior.moduleData), PilotFindVehicleKVMap);
    case Objects::ModuleType::POINT_DEFENSE_LASER:
      return parseSubtypedAttributeBlock<Objects::PointDefenseLaser>(std::move(behavior.moduleData), PointDefenseLaserKVMap);
    case Objects::ModuleType::POISONED:
      return parseSubtypedAttributeBlock<Objects::Poisoned>(std::move(behavior.moduleData), PoisonedKVMap);
    case Objects::ModuleType::POWER_PLANT:
      return parseSubtypedAttributeBlock<Objects::PowerPlant>(std::move(behavior.moduleData), PowerPlantKVMap);
    case Objects::ModuleType::PRODUCTION:
      return parseSubtypedAttributeBlock<Objects::Production>(std::move(behavior.moduleData), ProductionKVMap);
    case Objects::ModuleType::PROPAGANDA_TOWER:
      return parseSubtypedAttributeBlock<Objects::PropagandaTower>(std::move(behavior.moduleData), PropagandaTowerKVMap);
    case Objects::ModuleType::QUEUE_PRODUCTION_EXIT:
      return parseSubtypedAttributeBlock<Objects::QueueProductionExit>(std::move(behavior.moduleData), QueueProductionExitKVMap);
    case Objects::ModuleType::RADAR:
      return parseSubtypedAttributeBlock<Objects::Radar>(std::move(behavior.moduleData), RadarKVMap);
    case Objects::ModuleType::RAILED_TRANSPORT_AI:
      return parseSubtypedAttributeBlock<Objects::RailedTransportAI>(std::move(behavior.moduleData), RailedTransportAIKVMap);
    case Objects::ModuleType::RAILED_TRANSPORT_DOCK:
      return
        parseSubtypedAttributeBlocks<Objects::RailedTransportDock>(
            std::move(behavior.moduleData)
          , RailedTransportDockKVMap
          , DockKVMap
        );
    case Objects::ModuleType::RAILROAD:
      return
        parseSubtypedAttributeBlocks<Objects::RailroadBehavior>(
            std::move(behavior.moduleData)
          , RailroadBehaviorKVMap
          , PhysicsDataKVMap
        );
    case Objects::ModuleType::REBUILD_HOLE:
      return parseSubtypedAttributeBlock<Objects::RebuildHoleBehavior>(std::move(behavior.moduleData), RebuildHoleBehaviorKVMap);
    case Objects::ModuleType::REBUILD_HOLE_EXPOSE_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::RebuildHoleExposeDie>(
            std::move(behavior.moduleData)
          , RebuildHoleExposeDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::REPAIR_DOCK:
      return
        parseSubtypedAttributeBlocks<Objects::RepairDock>(
            std::move(behavior.moduleData)
          , RepairDockKVMap
          , DockKVMap
        );
    case Objects::ModuleType::REPLACE_OBJECT_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::ReplaceObjectUpgrade>(
            std::move(behavior.moduleData)
          , ReplaceObjectUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::SLAVED:
      return parseSubtypedAttributeBlock<Objects::Slaved>(std::move(behavior.moduleData), SlavedKVMap);
    case Objects::ModuleType::SLOW_DEATH:
      return
        parseSubtypedAttributeBlocks<Objects::SlowDeath>(
            std::move(behavior.moduleData)
          , SlowDeathKVMap
          , DieKVMap
        );
    case Objects::ModuleType::SPAWN:
      return parseSubtypedAttributeBlock<Objects::Spawn>(std::move(behavior.moduleData), SpawnKVMap);
    case Objects::ModuleType::SPAWN_POINT_PRODUCTION_EXIT:
      return parseSubtypedAttributeBlock<Objects::SpawnPointProductionExit>(std::move(behavior.moduleData), SpawnPointProductionExitKVMap);
    case Objects::ModuleType::SPECIAL_POWER:
      return parseSubtypedAttributeBlock<Objects::SpecialPower>(std::move(behavior.moduleData), SpecialPowerKVMap);
    case Objects::ModuleType::SPECIAL_POWER_UPDATE:
      return parseSubtypedAttributeBlock<Objects::SpecialPowerUpdate>(std::move(behavior.moduleData), SpecialPowerUpdateKVMap);
    case Objects::ModuleType::SPECTRE_GUNSHIP:
      return parseSubtypedAttributeBlock<Objects::SpectreGunship>(std::move(behavior.moduleData), SpectreGunshipKVMap);
    case Objects::ModuleType::SPECTRE_GUNSHIP_DEPLOYMENT:
      return parseSubtypedAttributeBlock<Objects::SpectreGunshipDeployment>(std::move(behavior.moduleData), SpectreGunshipDeploymentKVMap);
    case Objects::ModuleType::SPY_VISION:
      return
        parseSubtypedAttributeBlocks<Objects::SpyVision>(
            std::move(behavior.moduleData)
          , SpyVisionKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::SPY_VISION_SPECIAL_POWER:
      return
        parseSubtypedAttributeBlocks<Objects::SpyVisionSpecialPower>(
            std::move(behavior.moduleData)
          , SpyVisionSpecialPowerKVMap
          , SpecialPowerKVMap
        );
    case Objects::ModuleType::STEALTH:
      return parseSubtypedAttributeBlock<Objects::Stealth>(std::move(behavior.moduleData), StealthKVMap);
    case Objects::ModuleType::STEALTH_DETECTOR:
      return parseSubtypedAttributeBlock<Objects::StealthDetector>(std::move(behavior.moduleData), StealthDetectorKVMap);
    case Objects::ModuleType::STRUCTURE_COLLAPSE:
      return parseSubtypedAttributeBlock<Objects::StructureCollapse>(std::move(behavior.moduleData), StructureCollapseKVMap);
    case Objects::ModuleType::STRUCTURE_TOPPLE:
      return parseSubtypedAttributeBlock<Objects::StructureTopple>(std::move(behavior.moduleData), StructureToppleKVMap);
    case Objects::ModuleType::SUPPLY_CENTER_DOCK:
      return parseSubtypedAttributeBlocks<Objects::SupplyCenterDock>( std::move(behavior.moduleData) , DockKVMap);
    case Objects::ModuleType::SUPPLY_TRUCK_AI:
      return
        parseSubtypedAttributeBlocks<Objects::SupplyTruckAI>(
            std::move(behavior.moduleData)
          , SupplyTruckAIKVMap
          , AIKVMap
        );
    case Objects::ModuleType::SUPPLY_WAREHOUSE_CRIPPLING:
      return parseSubtypedAttributeBlock<Objects::SupplyWarehouseCrippling>(std::move(behavior.moduleData), SupplyWarehouseCripplingKVMap);
    case Objects::ModuleType::SUPPLY_WAREHOUSE_DOCK:
      return
        parseSubtypedAttributeBlocks<Objects::SupplyWarehouseDock>(
            std::move(behavior.moduleData)
          , SupplyWarehouseDockKVMap
          , DockKVMap
        );
    case Objects::ModuleType::TENSILE_FORMATION:
      return parseSubtypedAttributeBlock<Objects::TensileFormation>(std::move(behavior.moduleData), TensileFormationKVMap);
    case Objects::ModuleType::TOPPLE:
      return parseSubtypedAttributeBlock<Objects::Topple>(std::move(behavior.moduleData), ToppleKVMap);
    case Objects::ModuleType::TRANSITION_DAMAGE_FX:
      return parseSubtypedAttributeBlock<Objects::TransitionDamageFX>(std::move(behavior.moduleData), TransitionDamageFXKVMap);
    case Objects::ModuleType::INTERNET_HACK_CONTAIN:
    case Objects::ModuleType::RAILED_TRANSPORT_CONTAIN:
    case Objects::ModuleType::TRANSPORT_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::TransportContain>(
            std::move(behavior.moduleData)
          , TransportContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::TUNNEL_CONTAIN:
      return
        parseSubtypedAttributeBlocks<Objects::TunnelContain>(
            std::move(behavior.moduleData)
          , TunnelContainKVMap
          , OpenContainKVMap
        );
    case Objects::ModuleType::ARMOR_UPGRADE:
    case Objects::ModuleType::LOCOMOTOR_SET_UPGRADE:
    case Objects::ModuleType::RADAR_UPGRADE:
    case Objects::ModuleType::POWER_PLANT_UPGRADE:
    case Objects::ModuleType::STEALTH_UPGRADE:
    case Objects::ModuleType::UPGRADE:
    case Objects::ModuleType::WEAPON_BONUS_UPGRADE:
      return parseSubtypedAttributeBlocks<Objects::Upgrade>(std::move(behavior.moduleData), UpgradeKVMap<Objects::Upgrade>);
    case Objects::ModuleType::WEAPON_SET_UPGRADE:
      return parseSubtypedAttributeBlock<Objects::Upgrade>(std::move(behavior.moduleData), UpgradeKVMap<Objects::Upgrade>);
    case Objects::ModuleType::UPGRADE_DIE:
      return
        parseSubtypedAttributeBlocks<Objects::UpgradeDie>(
            std::move(behavior.moduleData)
          , UpgradeDieKVMap
          , DieKVMap
        );
    case Objects::ModuleType::UNPAUSE_SPECIAL_POWER_UPGRADE:
      return
        parseSubtypedAttributeBlocks<Objects::UnpauseSpecialPowerUpgrade>(
            std::move(behavior.moduleData)
          , UnpauseSpecialPowerUpgradeKVMap
          , UpgradeKVMap<Objects::Upgrade>
        );
    case Objects::ModuleType::VETERANCY_CRATE_COLLISION:
      return
        parseSubtypedAttributeBlocks<Objects::VeterancyCrateCollision>(
            std::move(behavior.moduleData)
          , VeterancyCrateCollisionKVMap
          , CrateCollisionKVMap
        );
    case Objects::ModuleType::VETERANCY_GAIN:
      return parseSubtypedAttributeBlock<Objects::VeterancyGain>(std::move(behavior.moduleData), VeterancyGainKVMap);
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as behavior: {}", values[0]);
      return false;
  }
}

bool ObjectsINI::parseBody(Objects::ObjectBuilder& builder) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  advanceStream();
  token = getTokenInLine();
  auto bodyType = Objects::getModuleType(token);
  if (!bodyType) {
    WARN_ZH("ObjectsINI", "Unsupported body type: {}", token);
    return false;
  }

  builder.body = std::make_shared<Objects::Behavior>();
  builder.body->type = *bodyType;

  advanceStream();
  builder.body->moduleTag = getTokenInLine();

  switch (builder.body->type) {
    case Objects::ModuleType::ACTIVE_BODY:
    case Objects::ModuleType::HIGHLANDER_BODY:
    case Objects::ModuleType::IMMORTAL_BODY:
    case Objects::ModuleType::STRUCTURE_BODY:
      return
        parseSubtypedAttributeBlock<Objects::ActiveBody>(
            std::move(builder.body->moduleData)
          , ActiveBodyKVMap
        );
    case Objects::ModuleType::HIVE_STRUCTURE_BODY:
      return
        parseSubtypedAttributeBlocks<Objects::HiveStructureBody>(
            std::move(builder.body->moduleData)
          , HiveStructureBodyKVMap
          , ActiveBodyKVMap
        );
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as body: {}", token);
      return false;
  }
}

bool ObjectsINI::parseClientUpdate(Objects::ObjectBuilder& builder) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  advanceStream();
  token = getTokenInLine();
  auto clientUpdateType = Objects::getModuleType(token);
  if (!clientUpdateType) {
    WARN_ZH("ObjectsINI", "Unsupported clientUpdate type: {}", token);
    return false;
  }

  builder.clientUpdate = std::make_shared<Objects::Behavior>();
  builder.clientUpdate->type = *clientUpdateType;

  advanceStream();
  builder.clientUpdate->moduleTag = getTokenInLine();

  switch (builder.clientUpdate->type) {
    case Objects::ModuleType::LASER:
      return
        parseSubtypedAttributeBlock<Objects::Laser>(
            std::move(builder.clientUpdate->moduleData)
          , LaserKVMap
        );
    case Objects::ModuleType::ANIMATED_PARTICLE_SYS_BONE_CLIENT:
    case Objects::ModuleType::SWAY_CLIENT:
      return parseEmptyAttributeBlock();
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as client update: {}", token);
      return false;
  }
}

// stuff found:
//   ConditionState = A B C
//   ConditionState A B
//   ConditionState
// so avoid breaking other things while dealing with this mess specifically
std::set<Objects::ModelCondition> ObjectsINI::parseConditionStateConditions() {
  advanceStreamInLine();
  auto token = getTokenInLine();

  if (token.empty()) {
    return {Objects::ModelCondition::NONE};
  }

  if (token == "=") {
    advanceStreamInLine();
    token = getTokenInLine();
  }

  std::set<Objects::ModelCondition> conditions;
  while (!token.empty()) {
    if (token == "None" || token == "NONE") {
      return {Objects::ModelCondition::NONE};
    }

    auto condOpt = Objects::getModelCondition(token);
    if (!condOpt) {
      WARN_ZH("ObjectsINI", "Model condition unknown: {}", token);
      continue;
    }

    conditions.emplace(*condOpt);

    advanceStreamInLine();
    token = getTokenInLine();
  }

  return conditions;
}

bool ObjectsINI::parseDraw(Objects::ObjectBuilder& builder) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  auto& metaData = builder.drawMetaData.emplace_back();

  advanceStream();
  token = getTokenInLine();
  if (token == "W3DDefaultDraw") {
    metaData.type = Objects::DrawType::DEFAULT_DRAW;
  } else if (token == "W3DDependencyModelDraw") {
    metaData.type = Objects::DrawType::DEPENDENCY_MODEL_DRAW;
  } else if (token == "W3DModelDraw") {
    metaData.type = Objects::DrawType::MODEL_DRAW;
  } else if (token == "W3DLaserDraw") {
    metaData.type = Objects::DrawType::LASER_DRAW;
  } else if (token == "W3DOverlordAircraftDraw") {
    metaData.type = Objects::DrawType::OVERLORD_AIRCRAFT_DRAW;
  } else if (token == "W3DOverlordTankDraw") {
    metaData.type = Objects::DrawType::OVERLORD_TANK_DRAW;
  } else if (token == "W3DOverlordTruckDraw") {
    metaData.type = Objects::DrawType::OVERLORD_TRUCK_DRAW;
  } else if (token == "W3DPoliceCarDraw") {
    metaData.type = Objects::DrawType::POLICE_CAR_DRAW;
  } else if (token == "W3DTankDraw") {
    metaData.type = Objects::DrawType::TANK_DRAW;
  } else if (token == "W3DTreeDraw") {
    metaData.type = Objects::DrawType::TREE_DRAW;
  } else if (token == "W3DTruckDraw") {
    metaData.type = Objects::DrawType::TRUCK_DRAW;
  } else if (token == "W3DSupplyDraw") {
    metaData.type = Objects::DrawType::SUPPLY_DRAW;
  } else {
    WARN_ZH("ObjectsINI", "Unsupported draw type {}", token);
  }

  advanceStream();
  metaData.moduleTag = getTokenInLine();

  switch (metaData.type) {
    case Objects::DrawType::DEFAULT_DRAW:
      return parseSubtypedAttributeBlocks<Objects::DrawData>(std::move(metaData.drawData));
    case Objects::DrawType::DEPENDENCY_MODEL_DRAW:
      return
        parseSubtypedAttributeBlocks<Objects::DependencyModelDrawData>(
            std::move(metaData.drawData)
          , DependencyModelDrawDataKVMap
          , ModelDrawDataKVMap
        );
    case Objects::DrawType::MODEL_DRAW:
    case Objects::DrawType::OVERLORD_AIRCRAFT_DRAW:
      return
        parseSubtypedAttributeBlock<Objects::ModelDrawData>(
            std::move(metaData.drawData)
          , ModelDrawDataKVMap
        );
    case Objects::DrawType::LASER_DRAW:
      return
        parseSubtypedAttributeBlock<Objects::LaserDrawData>(
            std::move(metaData.drawData)
          , LaserDrawDataKVMap
        );
    case Objects::DrawType::OVERLORD_TANK_DRAW:
    case Objects::DrawType::TANK_DRAW:
      return
        parseSubtypedAttributeBlocks<Objects::TankDrawData>(
            std::move(metaData.drawData)
          , TankDrawDataKVMap
          , ModelDrawDataKVMap
        );
    case Objects::DrawType::TREE_DRAW:
      return
        parseSubtypedAttributeBlock<Objects::TreeDrawData>(
            std::move(metaData.drawData)
          , TreeDrawDataKVMap
        );
    case Objects::DrawType::OVERLORD_TRUCK_DRAW:
    case Objects::DrawType::POLICE_CAR_DRAW:
    case Objects::DrawType::TRUCK_DRAW:
      return
        parseSubtypedAttributeBlocks<Objects::TruckDrawData>(
            std::move(metaData.drawData)
          , TruckDrawDataKVMap
          , ModelDrawDataKVMap
        );
    case Objects::DrawType::SUPPLY_DRAW:
      return
        parseSubtypedAttributeBlocks<Objects::SupplyDrawData>(
            std::move(metaData.drawData)
          , SupplyDrawDataKVMap
          , ModelDrawDataKVMap
        );
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as draw data: {}", token);
      return false;
  }
}

bool ObjectsINI::hasErroneousObject() const {
  return erroneousObject;
}

}
