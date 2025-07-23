#include "ObjectsINI.h"
#include "../Logging.h"

namespace ZH {

static INIApplierMap<Objects::ActiveBody> ActiveBodyDataKVMap = {
  { "InitialHealth", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.initialHealth = opt.value_or(1.0f);
      return opt.has_value();
    }
  },
  { "MaxHealth", [](Objects::ActiveBody& ab, INIFile& f) {
      auto opt = f.parseFloat();
      ab.maxHealth = opt.value_or(1.0f);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::ArmorSet> ArmorSetKVMap = {
  { "Armor", [](Objects::ArmorSet& as, INIFile& f) { as.armor = f.parseString(); return !as.armor.empty(); } },
  { "Conditions", [](Objects::ArmorSet& as, INIFile& f) {
      return f.parseEnumSet<Objects::ArmorSet::Condition>(as.conditions, &Objects::getArmorSetCondition);
    }
  },
  { "Damage", [](Objects::ArmorSet& as, INIFile& f) { as.damage = f.parseString(); return !as.damage.empty(); } }
};

static INIApplierMap<Objects::ConditionState> ConditionStateKVMap = {
  { "Model", [](Objects::ConditionState& cs, INIFile& f) { cs.model = f.parseString(); return !cs.model.empty(); } },
  { "ParticleSysBone", [](Objects::ConditionState& cs, INIFile& f) {
      auto value = f.parseStringList();
      if (value.size() != 2) {
        return false;
      }

      cs.particleBones.emplace_back(std::move(value));
      return true;
    }
  }
};

static INIApplierMap<Objects::FireWeaponCollision> FireWeaponCollisionDataKVMap = {
  { "CollideWeapon", [](Objects::FireWeaponCollision& c, INIFile& f) { c.weapon = f.parseString(); return !c.weapon.empty(); } },
  { "RequiredStatus", [](Objects::FireWeaponCollision& c, INIFile& f) {
      return f.parseEnumSet<Objects::Status>(c.requiredStates, &Objects::getStatus);
    }
  },
};

static INIApplierMap<Objects::FireSpread> FireSpreadDataKVMap = {
  { "OCLEmbers", [](Objects::FireSpread& fs, INIFile& f) { fs.creationList = f.parseString(); return !fs.creationList.empty(); } },
  { "MaxSpreadDelay", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseInteger();
      fs.maxSpreadDelayMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "MinSpreadDelay", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseInteger();
      fs.minSpreadDelayMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "SpreadTryRange", [](Objects::FireSpread& fs, INIFile& f) {
      auto opt = f.parseFloat();
      fs.spreadRange = opt.value_or(0.0f);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::Flammable> FlammableDataKVMap = {
  { "AflameDuration", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.burningDurationMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "BurnedDelay", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.burnedDelayMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "BurningSoundName", [](Objects::Flammable& fd, INIFile& f) { fd.sound = f.parseString(); return !fd.sound.empty(); } },
  { "FlameDamageExpiration", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.damageExpirationMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "FlameDamageLimit", [](Objects::Flammable& fd, INIFile& f) {
      auto opt = f.parseFloat();
      fd.damageLimit = opt.value_or(0.0f);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::Lifetime> LifetimeDataKVMap = {
  { "MaxLifetime", [](Objects::Lifetime& l, INIFile& f) {
      auto opt = f.parseInteger();
      l.maxLifetimeMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "MinLifetime", [](Objects::Lifetime& l, INIFile& f) {
      auto opt = f.parseInteger();
      l.minLifetimeMs = opt.value_or(1000);
      return opt.has_value();
    }
  }
};

static INIApplierMap<Objects::ModelDrawData> ModelDrawDataKVMap = {
  { "ConditionState", [](Objects::ModelDrawData& dd, INIFile& f) {
      auto& state = dd.conditionStates.emplace_back();
      state.tags = f.parseStringList();
      if (state.tags.empty()) {
        return false;
      }

      return f.parseAttributeBlock(state, ConditionStateKVMap);
    }
  },
  { "DefaultConditionState", [](Objects::ModelDrawData& dd, INIFile& f) {
      return f.parseAttributeBlock(dd.defaultConditionState, ConditionStateKVMap);
    }
  }
};

static INIApplierMap<Objects::Physics> PhysicsDataKVMap = {
  { "Mass", [](Objects::Physics& p, INIFile& f) {
      auto opt = f.parseFloat();
      p.mass = opt.value_or(0.0f);
      return opt.has_value();
    }
  },
  { "KillWhenRestingOnGround", [](Objects::Physics& p, INIFile& f) { p.killOnGround = f.parseBool(); return true; } },
};

static INIApplierMap<Objects::ObjectBuilder> ObjectDataKVMap = {
  { "ArmorSet", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto& armorSet = b.armorSets.emplace_back();
      return f.parseAttributeBlock(armorSet, ArmorSetKVMap);
    }
  },
  { "Behavior", [](Objects::ObjectBuilder& b, INIFile& f) {
      return reinterpret_cast<ObjectsINI&>(f).parseBehavior(b);
    }
  },
  { "Body", [](Objects::ObjectBuilder& b, INIFile& f) {
      return reinterpret_cast<ObjectsINI&>(f).parseBody(b);
    }
  },
  { "ClientUpdate", [](Objects::ObjectBuilder& b, INIFile& f) {
      return reinterpret_cast<ObjectsINI&>(f).parseClientUpdate(b);
    }
  },
  { "CrushableLevel", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseByte();
      b.crushableLevel = value.value_or(0);
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
  { "Draw", [](Objects::ObjectBuilder& b, INIFile& f) {
      return reinterpret_cast<ObjectsINI&>(f).parseDraw(b);
    }
  },
  { "EditorSorting", [](Objects::ObjectBuilder& b, INIFile& f) { f.parseString(); return true; } },
  { "FenceWidth", [](Objects::ObjectBuilder& b, INIFile& f) { f.parseFloat(); return true; } },
  { "FenceXOffset", [](Objects::ObjectBuilder& b, INIFile& f) { f.parseFloat(); return true; } },
  { "Geometry", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseEnum<Objects::Geometry>(&Objects::getGeometry);
      b.geometry.type = opt.value_or(Objects::Geometry::NONE);
      return opt.has_value();
    }
  },
  { "GeometryMajorRadius", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.majorRadius = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "GeometryMinorRadius", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.minorRadius = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "GeometryHeight", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.geometry.height = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "GeometryIsSmall", [](Objects::ObjectBuilder& b, INIFile& f) { b.geometry.small = f.parseBool(); return true; } },
  { "InstanceScaleFuzziness", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto value = f.parseFloat();
      b.scaleFuzziness = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "KindOf", [](Objects::ObjectBuilder& b, INIFile& f) {
      return f.parseEnumSet<Objects::Attribute>(b.attributes, &Objects::getAttribute);
    }
  },
  { "Shadow", [](Objects::ObjectBuilder& b, INIFile& f) {
      auto opt = f.parseEnum<Objects::Shadow>(&Objects::getShadow);
      b.shadow.type = opt.value_or(Objects::Shadow::NONE);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::SlowDeath> SlowDeathDataKVMap = {
  { "DestructionDelay", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = f.parseInteger();
      sd.destructionDelayMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "DestructionDelayVariance", [](Objects::SlowDeath& sd, INIFile& f) {
      auto opt = f.parseInteger();
      sd.destructionDelayVarianceMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "DeathTypes", [](Objects::SlowDeath& sd, INIFile& f) {
      return f.parseEnumSet<Objects::DeathType>(sd.deathTypes, &Objects::getDeathType);
    }
  },
  { "SinkRate", [](Objects::SlowDeath& sd, INIFile& f) {
      auto value = f.parseFloat();
      sd.sinkRate = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "SinkDelay", [](Objects::SlowDeath& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.sinkDelayMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "SinkDelayVariance", [](Objects::SlowDeath& fd, INIFile& f) {
      auto opt = f.parseInteger();
      fd.sinkDelayVarianceMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
};

static INIApplierMap<Objects::Topple> ToppleDataKVMap = {
  { "BounceFX", [](Objects::Topple& t, INIFile& f) { t.bounceEffect = f.parseString(); return !t.bounceEffect.empty(); } },
  { "BounceVelocityPercent", [](Objects::Topple& t, INIFile& f) {
      auto opt = f.parsePercent();
      t.bounceVelocity = opt.value_or(0);
      return opt.has_value();
    }
  },
  { "InitialAccelPercent", [](Objects::Topple& t, INIFile& f) {
      auto opt = f.parsePercent();
      t.initialAcceleration = opt.value_or(0);
      return opt.has_value();
    }
  },
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
      t.sinkDistance = value.value_or(1.0f);
      return value.has_value();
    }
  },
  { "SinkTime", [](Objects::TreeDrawData& t, INIFile& f) {
      auto opt = f.parseInteger();
      t.sinkTimeMs = opt.value_or(1000);
      return opt.has_value();
    }
  },
  { "StumpName", [](Objects::TreeDrawData& t, INIFile& f) { t.stump = f.parseString(); return !t.stump.empty(); } },
  { "TextureName", [](Objects::TreeDrawData& t, INIFile& f) { t.texture = f.parseString(); return !t.texture.empty(); } },
  { "ToppleFX", [](Objects::TreeDrawData& t, INIFile& f) { t.toppleEffect = f.parseString(); return !t.toppleEffect.empty(); } },
};

ObjectsINI::ObjectsINI(std::istream& stream) : INIFile(stream) {}

ObjectsINI::ObjectMap ObjectsINI::parse() {
  ObjectMap objects;

  while (!stream.eof()) {
    parseObject(objects);
  }

  return objects;
}

bool ObjectsINI::parseObject(ObjectMap& objects) {
  auto token = consumeComment();

  if (token != "ObjectReskin" && token != "Object") {
    return false;
  }
  bool reskinning = token == "ObjectReskin";

  advanceStream();
  auto key = getTokenInLine();

  Objects::ObjectBuilder builder;

  if (reskinning) {
    advanceStream();
    auto reskinFrom = getTokenInLine();

    auto lookup = objects.find(reskinFrom);
    if (lookup != objects.cend()) {
      builder = *lookup->second;
    } else {
      WARN_ZH("ObjectsINI", "Cannot reskin from {}", reskinFrom);
    }
  }

  if (parseAttributeBlock(builder, ObjectDataKVMap)) {
    objects.emplace(
        std::move(key)
      , std::make_shared<Objects::ObjectBuilder>(std::move(builder))
    );
    return true;
  } else {
    return false;
  }
}

bool ObjectsINI::parseBehavior(Objects::ObjectBuilder& builder) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  advanceStream();
  token = getTokenInLine();
  auto behaviorType = Objects::getModuleType(token);
  if (!behaviorType) {
    WARN_ZH("ObjectsINI", "Unsupported body type: {}", token);
    return false;
  }

  auto& behavior = builder.behaviors.emplace_back();
  behavior.type = *behaviorType;

  advanceStream();
  behavior.moduleTag = getTokenInLine();

  bool success = false;
  switch (behavior.type) {
    case Objects::ModuleType::FIRE_SPREAD:
      success = parseSubtypedAttributeBlock<Objects::FireSpread>(std::move(behavior.moduleData), FireSpreadDataKVMap);
      break;
    case Objects::ModuleType::FIRE_WEAPON_COLLISION:
      success =
        parseSubtypedAttributeBlock<Objects::FireWeaponCollision>(
            std::move(behavior.moduleData)
          , FireWeaponCollisionDataKVMap
        );
      break;
    case Objects::ModuleType::FLAMMABLE:
      success = parseSubtypedAttributeBlock<Objects::Flammable>(std::move(behavior.moduleData), FlammableDataKVMap);
      break;
    case Objects::ModuleType::LIFETIME:
      success = parseSubtypedAttributeBlock<Objects::Lifetime>(std::move(behavior.moduleData), LifetimeDataKVMap);
      break;
    case Objects::ModuleType::PHYSICS:
      success = parseSubtypedAttributeBlock<Objects::Physics>(std::move(behavior.moduleData), PhysicsDataKVMap);
      break;
    case Objects::ModuleType::TOPPLE:
      success = parseSubtypedAttributeBlock<Objects::Topple>(std::move(behavior.moduleData), ToppleDataKVMap);
      break;
    case Objects::ModuleType::SLOW_DEATH:
      success = parseSubtypedAttributeBlock<Objects::SlowDeath>(std::move(behavior.moduleData), SlowDeathDataKVMap);
      break;
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as behavior: {}", token);
      return false;
  }

  return success;
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

  bool success = false;
  switch (builder.body->type) {
    case Objects::ModuleType::ACTIVE_BODY:
    case Objects::ModuleType::HIGHLANDER_BODY:
      success =
        parseSubtypedAttributeBlock<Objects::ActiveBody>(
            std::move(builder.body->moduleData)
          , ActiveBodyDataKVMap
        );
      break;
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as body: {}", token);
      return false;
  }

  return success;
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

  bool success = false;
  switch (builder.clientUpdate->type) {
    case Objects::ModuleType::SWAY_CLIENT:
      success = parseEmptyAttributeBlock();
      break;
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as client update: {}", token);
      return false;
  }

  return success;
}

bool ObjectsINI::parseDraw(Objects::ObjectBuilder& builder) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  advanceStream();
  token = getTokenInLine();
  if (token == "W3DModelDraw") {
    builder.drawMetaData.type = Objects::DrawType::MODEL_DRAW;
  } else if (token == "W3DTreeDraw") {
    builder.drawMetaData.type = Objects::DrawType::TREE_DRAW;
  } else {
    WARN_ZH("ObjectsINI", "Unsupported draw type {}", token);
  }

  advanceStream();
  builder.drawMetaData.moduleTag = getTokenInLine();

  bool success = false;
  switch (builder.drawMetaData.type) {
    case Objects::DrawType::MODEL_DRAW:
      success =
        parseSubtypedAttributeBlock<Objects::ModelDrawData>(
            std::move(builder.drawMetaData.drawData)
          , ModelDrawDataKVMap
        );
      break;
    case Objects::DrawType::TREE_DRAW:
      success =
        parseSubtypedAttributeBlock<Objects::TreeDrawData>(
            std::move(builder.drawMetaData.drawData)
          , TreeDrawDataKVMap
        );
      break;
    default:
      WARN_ZH("ObjectsINI", "Module type not supported as draw data: {}", token);
      return false;
  }

  return success;
}

}
