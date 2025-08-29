#include "Object.h"

namespace ZH::Objects {

std::optional<AnimationMode> getAnimationMode(const std::string_view& value) {
  if (value == "NONE") {
    return {AnimationMode::NONE};
  } else if (value == "ONCE") {
    return {AnimationMode::ONCE};
  } else if (value == "ONCE_BACKWARDS") {
    return {AnimationMode::ONCE_BACKWARDS};
  } else if (value == "LOOP") {
    return {AnimationMode::LOOP};
  } else if (value == "LOOP_BACKWARDS") {
    return {AnimationMode::LOOP_BACKWARDS};
  } else if (value == "MANUAL") {
    return {AnimationMode::MANUAL};
  } else if (value == "PING_PONG") {
    return {AnimationMode::BACK_AND_FORTH};
  } else if (value == "PING_PONG_BACKWARDS") {
    return {AnimationMode::FORTH_AND_BACK};
  } else {
    return {};
  }
}

std::optional<AnimationFrameMode> getAnimationFrameMode(const std::string_view& value) {
  if (value == "RANDOMSTART") {
    return {AnimationFrameMode::RANDOM};
  } else if (value == "START_FRAME_FIRST") {
    return {AnimationFrameMode::START_FRAME_FIRST};
  } else if (value == "START_FRAME_LAST") {
    return {AnimationFrameMode::START_FRAME_LAST};
  } else if (value == "ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT") {
    return {AnimationFrameMode::ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT};
  } else if (value == "PRISTINE_BONE_POS_IN_FINAL_FRAME") {
    return {AnimationFrameMode::PRISTINE_BONE_POS_IN_FINAL_FRAME};
  } else if (value == "MAINTAIN_FRAME_ACROSS_STATES") {
    return {AnimationFrameMode::MAINTAIN_FRAME_ACROSS_STATES};
  } else if (value == "MAINTAIN_FRAME_ACROSS_STATES2") {
    return {AnimationFrameMode::MAINTAIN_FRAME_ACROSS_STATES2};
  } else if (value == "MAINTAIN_FRAME_ACROSS_STATES3") {
    return {AnimationFrameMode::MAINTAIN_FRAME_ACROSS_STATES3};
  } else if (value == "MAINTAIN_FRAME_ACROSS_STATES4") {
    return {AnimationFrameMode::MAINTAIN_FRAME_ACROSS_STATES4};
  } else if (value == "RESTART_ANIM_WHEN_COMPLETE") {
    return {AnimationFrameMode::RESTART_ANIM_WHEN_COMPLETE};
  } else {
    return {};
  }
}

std::optional<ArmorSet::Condition> getArmorSetCondition(const std::string_view& value) {
 if (value == "VETERAN") {
   return {ArmorSet::Condition::VETERAN};
 } else if (value == "ELITE") {
   return {ArmorSet::Condition::ELITE};
 } else if (value == "HERO") {
   return {ArmorSet::Condition::HERO};
 } else if (value == "PLAYER_UPGRADE") {
   return {ArmorSet::Condition::PLAYER_UPGRADE};
 } else if (value == "WEAK_VERSUS_BASEDEFENSES") {
   return {ArmorSet::Condition::WEAK_VS_BASE_DEFENSE};
 } else if (value == "SECOND_LIFE") {
   return {ArmorSet::Condition::SECOND_LIFE};
 } else if (value == "CRATE_UPGRADE_ONE") {
   return {ArmorSet::Condition::CRATE_UPGRADE_ONE};
 } else if (value == "CRATE_UPGRADE_TWO") {
   return {ArmorSet::Condition::CRATE_UPGRADE_TWO};
 } else {
   return {};
 }
}

std::optional<Attribute> getAttribute(const std::string_view& value) {
  if (value == "OBSTACLE") {
    return {Attribute::OBSTACLE};
  } else if (value == "SELECTABLE") {
    return {Attribute::SELECTABLE};
  } else if (value == "IMMOBILE") {
    return {Attribute::IMMOBILE};
  } else if (value == "CAN_ATTACK") {
    return {Attribute::CAN_ATTACK};
  } else if (value == "STICK_TO_TERRAIN_SLOPE") {
    return {Attribute::STICK_TO_TERRAIN_SLOPE};
  } else if (value == "CAN_CAST_REFLECTIONS") {
    return {Attribute::CAN_CAST_REFLECTIONS};
  } else if (value == "SHRUBBERY") {
    return {Attribute::SHRUBBERY};
  } else if (value == "STRUCTURE") {
    return {Attribute::STRUCTURE};
  } else if (value == "INFANTRY") {
    return {Attribute::INFANTRY};
  } else if (value == "VEHICLE") {
    return {Attribute::VEHICLE};
  } else if (value == "AIRCRAFT") {
    return {Attribute::AIRCRAFT};
  } else if (value == "HUGE_VEHICLE") {
    return {Attribute::HUGE_VEHICLE};
  } else if (value == "DOZER") {
    return {Attribute::DOZER};
  } else if (value == "HARVESTER") {
    return {Attribute::HARVESTER};
  } else if (value == "COMMANDCENTER") {
    return {Attribute::COMMANDCENTER};
  } else if (value == "LINEBUILD") {
    return {Attribute::LINEBUILD};
  } else if (value == "SALVAGER") {
    return {Attribute::SALVAGER};
  } else if (value == "WEAPON_SALVAGER") {
    return {Attribute::WEAPON_SALVAGER};
  } else if (value == "TRANSPORT") {
    return {Attribute::TRANSPORT};
  } else if (value == "BRIDGE") {
    return {Attribute::BRIDGE};
  } else if (value == "LANDMARK_BRIDGE") {
    return {Attribute::LANDMARK_BRIDGE};
  } else if (value == "BRIDGE_TOWER") {
    return {Attribute::BRIDGE_TOWER};
  } else if (value == "PROJECTILE") {
    return {Attribute::PROJECTILE};
  } else if (value == "PRELOAD") {
    return {Attribute::PRELOAD};
  } else if (value == "NO_GARRISON") {
    return {Attribute::NO_GARRISON};
  } else if (value == "WAVEGUIDE") {
    return {Attribute::WAVEGUIDE};
  } else if (value == "WAVE_EFFECT") {
    return {Attribute::WAVE_EFFECT};
  } else if (value == "NO_COLLIDE") {
    return {Attribute::NO_COLLIDE};
  } else if (value == "REPAIR_PAD") {
    return {Attribute::REPAIR_PAD};
  } else if (value == "HEAL_PAD") {
    return {Attribute::HEAL_PAD};
  } else if (value == "STEALTH_GARRISON") {
    return {Attribute::STEALTH_GARRISON};
  } else if (value == "CASH_GENERATOR") {
    return {Attribute::CASH_GENERATOR};
  } else if (value == "DRAWABLE_ONLY") {
    return {Attribute::DRAWABLE_ONLY};
  } else if (value == "MP_COUNT_FOR_VICTORY") {
    return {Attribute::MP_COUNT_FOR_VICTORY};
  } else if (value == "REBUILD_HOLE") {
    return {Attribute::REBUILD_HOLE};
  } else if (value == "SCORE") {
    return {Attribute::SCORE};
  } else if (value == "SCORE_CREATE") {
    return {Attribute::SCORE_CREATE};
  } else if (value == "SCORE_DESTROY") {
    return {Attribute::SCORE_DESTROY};
  } else if (value == "NO_HEAL_ICON") {
    return {Attribute::NO_HEAL_ICON};
  } else if (value == "CAN_RAPPEL") {
    return {Attribute::CAN_RAPPEL};
  } else if (value == "PARACHUTABLE") {
    return {Attribute::PARACHUTABLE};
  } else if (value == "CAN_BE_REPULSED") {
    return {Attribute::CAN_BE_REPULSED};
  } else if (value == "MOB_NEXUS") {
    return {Attribute::MOB_NEXUS};
  } else if (value == "IGNORED_IN_GUI") {
    return {Attribute::IGNORED_IN_GUI};
  } else if (value == "CRATE") {
    return {Attribute::CRATE};
  } else if (value == "CAPTURABLE") {
    return {Attribute::CAPTURABLE};
  } else if (value == "CLEARED_BY_BUILD") {
    return {Attribute::CLEARED_BY_BUILD};
  } else if (value == "SMALL_MISSILE") {
    return {Attribute::SMALL_MISSILE};
  } else if (value == "ALWAYS_VISIBLE") {
    return {Attribute::ALWAYS_VISIBLE};
  } else if (value == "UNATTACKABLE") {
    return {Attribute::UNATTACKABLE};
  } else if (value == "MINE") {
    return {Attribute::MINE};
  } else if (value == "CLEANUP_HAZARD") {
    return {Attribute::CLEANUP_HAZARD};
  } else if (value == "PORTABLE_STRUCTURE") {
    return {Attribute::PORTABLE_STRUCTURE};
  } else if (value == "ALWAYS_SELECTABLE") {
    return {Attribute::ALWAYS_SELECTABLE};
  } else if (value == "ATTACK_NEEDS_LINE_OF_SIGHT") {
    return {Attribute::ATTACK_NEEDS_LINE_OF_SIGHT};
  } else if (value == "WALK_ON_TOP_OF_WALL") {
    return {Attribute::WALK_ON_TOP_OF_WALL};
  } else if (value == "DEFENSIVE_WALL") {
    return {Attribute::DEFENSIVE_WALL};
  } else if (value == "FS_POWER") {
    return {Attribute::FS_POWER};
  } else if (value == "FS_FACTORY") {
    return {Attribute::FS_FACTORY};
  } else if (value == "FS_BASE_DEFENSE") {
    return {Attribute::FS_BASE_DEFENSE};
  } else if (value == "FS_TECHNOLOGY") {
    return {Attribute::FS_TECHNOLOGY};
  } else if (value == "AIRCRAFT_PATH_AROUND") {
    return {Attribute::AIRCRAFT_PATH_AROUND};
  } else if (value == "LOW_OVERLAPPABLE") {
    return {Attribute::LOW_OVERLAPPABLE};
  } else if (value == "FORCEATTACKABLE") {
    return {Attribute::FORCEATTACKABLE};
  } else if (value == "AUTO_RALLYPOINT") {
    return {Attribute::AUTO_RALLYPOINT};
  } else if (value == "TECH_BUILDING") {
    return {Attribute::TECH_BUILDING};
  } else if (value == "POWERED") {
    return {Attribute::POWERED};
  } else if (value == "PRODUCED_AT_HELIPAD") {
    return {Attribute::PRODUCED_AT_HELIPAD};
  } else if (value == "DRONE") {
    return {Attribute::DRONE};
  } else if (value == "CAN_SEE_THROUGH_STRUCTURE") {
    return {Attribute::CAN_SEE_THROUGH_STRUCTURE};
  } else if (value == "BALLISTIC_MISSILE") {
    return {Attribute::BALLISTIC_MISSILE};
  } else if (value == "CLICK_THROUGH") {
    return {Attribute::CLICK_THROUGH};
  } else if (value == "SUPPLY_SOURCE_ON_PREVIEW") {
    return {Attribute::SUPPLY_SOURCE_ON_PREVIEW};
  } else if (value == "PARACHUTE") {
    return {Attribute::PARACHUTE};
  } else if (value == "GARRISONABLE_UNTIL_DESTROYED") {
    return {Attribute::GARRISONABLE_UNTIL_DESTROYED};
  } else if (value == "BOAT") {
    return {Attribute::BOAT};
  } else if (value == "IMMUNE_TO_CAPTURE") {
    return {Attribute::IMMUNE_TO_CAPTURE};
  } else if (value == "HULK") {
    return {Attribute::HULK};
  } else if (value == "SHOW_PORTRAIT_WHEN_CONTROLLED") {
    return {Attribute::SHOW_PORTRAIT_WHEN_CONTROLLED};
  } else if (value == "SPAWNS_ARE_THE_WEAPONS") {
    return {Attribute::SPAWNS_ARE_THE_WEAPONS};
  } else if (value == "CANNOT_BUILD_NEAR_SUPPLIES") {
    return {Attribute::CANNOT_BUILD_NEAR_SUPPLIES};
  } else if (value == "SUPPLY_SOURCE") {
    return {Attribute::SUPPLY_SOURCE};
  } else if (value == "REVEAL_TO_ALL") {
    return {Attribute::REVEAL_TO_ALL};
  } else if (value == "DISGUISER") {
    return {Attribute::DISGUISER};
  } else if (value == "INERT") {
    return {Attribute::INERT};
  } else if (value == "HERO") {
    return {Attribute::HERO};
  } else if (value == "IGNORES_SELECT_ALL") {
    return {Attribute::IGNORES_SELECT_ALL};
  } else if (value == "DONT_AUTO_CRUSH_INFANTRY") {
    return {Attribute::DONT_AUTO_CRUSH_INFANTRY};
  } else if (value == "CLIFF_JUMPER") {
    return {Attribute::CLIFF_JUMPER};
  } else if (value == "FS_SUPPLY_DROPZONE") {
    return {Attribute::FS_SUPPLY_DROPZONE};
  } else if (value == "FS_SUPERWEAPON") {
    return {Attribute::FS_SUPERWEAPON};
  } else if (value == "FS_BLACK_MARKET") {
    return {Attribute::FS_BLACK_MARKET};
  } else if (value == "FS_SUPPLY_CENTER") {
    return {Attribute::FS_SUPPLY_CENTER};
  } else if (value == "FS_STRATEGY_CENTER") {
    return {Attribute::FS_STRATEGY_CENTER};
  } else if (value == "MONEY_HACKER") {
    return {Attribute::MONEY_HACKER};
  } else if (value == "ARMOR_SALVAGER") {
    return {Attribute::ARMOR_SALVAGER};
  } else if (value == "REVEALS_ENEMY_PATHS") {
    return {Attribute::REVEALS_ENEMY_PATHS};
  } else if (value == "BOOBY_TRAP") {
    return {Attribute::BOOBY_TRAP};
  } else if (value == "FS_FAKE") {
    return {Attribute::FS_FAKE};
  } else if (value == "FS_INTERNET_CENTER") {
    return {Attribute::FS_INTERNET_CENTER};
  } else if (value == "BLAST_CRATER") {
    return {Attribute::BLAST_CRATER};
  } else if (value == "PROP") {
    return {Attribute::PROP};
  } else if (value == "OPTIMIZED_TREE") {
    return {Attribute::OPTIMIZED_TREE};
  } else if (value == "FS_ADVANCED_TECH") {
    return {Attribute::FS_ADVANCED_TECH};
  } else if (value == "FS_BARRACKS") {
    return {Attribute::FS_BARRACKS};
  } else if (value == "FS_WARFACTORY") {
    return {Attribute::FS_WARFACTORY};
  } else if (value == "FS_AIRFIELD") {
    return {Attribute::FS_AIRFIELD};
  } else if (value == "AIRCRAFT_CARRIER") {
    return {Attribute::AIRCRAFT_CARRIER};
  } else if (value == "NO_SELECT") {
    return {Attribute::NO_SELECT};
  } else if (value == "REJECT_UNMANNED") {
    return {Attribute::REJECT_UNMANNED};
  } else if (value == "CANNOT_RETALIATE") {
    return {Attribute::CANNOT_RETALIATE};
  } else if (value == "TECH_BASE_DEFENSE") {
    return {Attribute::TECH_BASE_DEFENSE};
  } else if (value == "EMP_HARDENED") {
    return {Attribute::EMP_HARDENED};
  } else if (value == "DEMOTRAP") {
    return {Attribute::DEMOTRAP};
  } else if (value == "CONSERVATIVE_BUILDING") {
    return {Attribute::CONSERVATIVE_BUILDING};
  } else if (value == "IGNORE_DOCKING_BONES") {
    return {Attribute::IGNORE_DOCKING_BONES};
  } else {
    return {};
  }
}

std::optional<AutoAcquireEnemyMode> getAutoAcquireEnemyMode(const std::string_view& value) {
  if (value == "YES") {
    return {AutoAcquireEnemyMode::YES};
  } else if (value == "NO") {
    return {AutoAcquireEnemyMode::NO};
  } else if (value == "STEALTHED") {
    return {AutoAcquireEnemyMode::STEALTHED};
  } else if (value == "NOTWHILEATTACKING") {
    return {AutoAcquireEnemyMode::NOT_WHILE_ATTACKING};
  } else if (value == "ATTACK_BUILDINGS") {
    return {AutoAcquireEnemyMode::ATTACK_BUILDINGS};
  } else {
    return {};
  }
}

std::optional<CompletionAppearance> getCompletionAppearance(const std::string_view& value) {
  if (value == "APPEARS_AT_RALLY_POINT") {
    return {CompletionAppearance::RALLY_POINT};
  } else if (value == "PLACED_BY_PLAYER") {
    return {CompletionAppearance::PLACEMENT};
  } else {
    return {};
  }
}

std::optional<CommandSource> getCommandSource(const std::string_view& value) {
  if (value == "NONE") {
    return {CommandSource::NONE};
  } if (value == "FROM_PLAYER") {
    return {CommandSource::PLAYER};
  } else if (value == "FROM_SCRIPT") {
    return {CommandSource::SCRIPT};
  } else if (value == "FROM_AI") {
    return {CommandSource::AI};
  } else if (value == "DEFAULT_SWITCH_WEAPON") {
    return {CommandSource::FALLBACK};
  } else {
    return {};
  }
}

std::optional<DamageType> getDamageType(const std::string_view& value) {
  if (value == "EXPLOSION") {
    return {DamageType::EXPLOSION};
  } else if (value == "CRUSH") {
    return {DamageType::CRUSH};
  } else if (value == "ARMOR_PIERCING") {
    return {DamageType::ARMOR_PIERCING};
  } else if (value == "SMALL_ARMS") {
    return {DamageType::SMALL_ARMS};
  } else if (value == "GATTLING") {
    return {DamageType::GATTLING};
  } else if (value == "RADIATION") {
    return {DamageType::RADIATION};
  } else if (value == "FIRE") {
    return {DamageType::FIRE};
  } else if (value == "LASER") {
    return {DamageType::LASER};
  } else if (value == "SNIPER") {
    return {DamageType::SNIPER};
  } else if (value == "POISON") {
    return {DamageType::POISON};
  } else if (value == "HEALING") {
    return {DamageType::HEALING};
  } else if (value == "UNRESISTABLE") {
    return {DamageType::UNRESISTABLE};
  } else if (value == "WATER") {
    return {DamageType::WATER};
  } else if (value == "DEPLOY") {
    return {DamageType::DEPLOY};
  } else if (value == "SURRENDER") {
    return {DamageType::SURRENDER};
  } else if (value == "HACK") {
    return {DamageType::HACK};
  } else if (value == "KILLPILOT") {
    return {DamageType::KILLPILOT};
  } else if (value == "PENALTY") {
    return {DamageType::PENALTY};
  } else if (value == "FALLING") {
    return {DamageType::FALLING};
  } else if (value == "MELEE") {
    return {DamageType::MELEE};
  } else if (value == "DISARM") {
    return {DamageType::DISARM};
  } else if (value == "HAZARD_CLEANUP") {
    return {DamageType::HAZARD_CLEANUP};
  } else if (value == "PARTICLE_BEAM") {
    return {DamageType::PARTICLE_BEAM};
  } else if (value == "TOPPLING") {
    return {DamageType::TOPPLING};
  } else if (value == "INFANTRY_MISSILE") {
    return {DamageType::INFANTRY_MISSILE};
  } else if (value == "AURORA_BOMB") {
    return {DamageType::AURORA_BOMB};
  } else if (value == "LAND_MINE") {
    return {DamageType::LAND_MINE};
  } else if (value == "JET_MISSILES") {
    return {DamageType::JET_MISSILES};
  } else if (value == "STEALTHJET_MISSILES") {
    return {DamageType::STEALTHJET_MISSILES};
  } else if (value == "MOLOTOV_COCKTAIL") {
    return {DamageType::MOLOTOV_COCKTAIL};
  } else if (value == "COMANCHE_VULCAN") {
    return {DamageType::COMANCHE_VULCAN};
  } else if (value == "SUBDUAL_MISSILE") {
    return {DamageType::SUBDUAL_MISSILE};
  } else if (value == "SUBDUAL_VEHICLE") {
    return {DamageType::SUBDUAL_VEHICLE};
  } else if (value == "SUBDUAL_BUILDING") {
    return {DamageType::SUBDUAL_BUILDING};
  } else if (value == "SUBDUAL_UNRESISTABLE") {
    return {DamageType::SUBDUAL_UNRESISTABLE};
  } else if (value == "MICROWAVE") {
    return {DamageType::MICROWAVE};
  } else if (value == "KILL_GARRISONED") {
    return {DamageType::KILL_GARRISONED};
  } else if (value == "STATUS") {
    return {DamageType::STATUS};
  } else {
    return {};
  }
}

std::optional<DeathType> getDeathType(const std::string_view& value) {
  if (value == "NORMAL") {
    return {DeathType::NORMAL};
  } else if (value == "CRUSHED") {
    return {DeathType::CRUSHED};
  } else if (value == "BURNED") {
    return {DeathType::BURNED};
  } else if (value == "EXPLODED") {
    return {DeathType::EXPLODED};
  } else if (value == "POISONED") {
    return {DeathType::POISONED};
  } else if (value == "TOPPLED") {
    return {DeathType::TOPPLED};
  } else if (value == "FLOODED") {
    return {DeathType::FLOODED};
  } else if (value == "SUICIDED") {
    return {DeathType::SUICIDED};
  } else if (value == "LASERED") {
    return {DeathType::LASERED};
  } else if (value == "DETONATED") {
    return {DeathType::DETONATED};
  } else if (value == "SPLATTED") {
    return {DeathType::SPLATTED};
  } else if (value == "POISONED_BETA") {
    return {DeathType::POISONED_BETA};
  } else if (value == "EXTRA_2") {
    return {DeathType::EXTRA_2};
  } else if (value == "EXTRA_3") {
    return {DeathType::EXTRA_3};
  } else if (value == "EXTRA_4") {
    return {DeathType::EXTRA_4};
  } else if (value == "EXTRA_5") {
    return {DeathType::EXTRA_5};
  } else if (value == "EXTRA_6") {
    return {DeathType::EXTRA_6};
  } else if (value == "EXTRA_7") {
    return {DeathType::EXTRA_7};
  } else if (value == "EXTRA_8") {
    return {DeathType::EXTRA_8};
  } else if (value == "POISONED_GAMMA") {
    return {DeathType::POISONED_GAMMA};
  } else {
    return {};
  }
}

std::optional<Geometry> getGeometry(const std::string_view& value) {
  if (value == "BOX") {
    return {Geometry::BOX};
  } else if (value == "CYLINDER") {
    return {Geometry::CYLINDER};
  } else if (value == "SPHERE") {
    return {Geometry::SPHERE};
  } else {
    return {};
  }
}

std::optional<LocomotorType> getLocomotorType(const std::string_view& value) {
  if (value == "SET_NORMAL") {
    return {Objects::LocomotorType::NORMAL};
  } else if (value == "SET_NORMAL_UPGRADED") {
    return {Objects::LocomotorType::NORMAL_UPGRADED};
  } else if (value == "SET_FREEFALL") {
    return {Objects::LocomotorType::FREEFALL};
  } else if (value == "SET_WANDER") {
    return {Objects::LocomotorType::WANDER};
  } else if (value == "SET_PANIC") {
    return {Objects::LocomotorType::PANIC};
  } else if (value == "SET_TAXIING") {
    return {Objects::LocomotorType::TAXIING};
  } else if (value == "SET_SUPERSONIC") {
    return {Objects::LocomotorType::SUPERSONIC};
  } else if (value == "SET_SLUGGISH") {
    return {Objects::LocomotorType::SLUGGISH};
  } else {
    return {};
  }
}

std::optional<MaxHealthModifier> getMaxHealthModifier(const std::string_view& value) {
  if (value == "SAME_CURRENTHEALTH") {
    return {MaxHealthModifier::SAME};
  } else if (value == "PRESERVE_RATIO") {
    return {MaxHealthModifier::PRESERVE_RATIO};
  } else if (value == "ADD_CURRENT_HEALTH_TOO") {
    return {MaxHealthModifier::ADD};
  } else if (value == "FULLY_HEAL") {
    return {MaxHealthModifier::FULLY_HEAL};
  } else {
    return {};
  }
}

std::optional<ModelCondition> getModelCondition(const std::string_view& value) {
  if (value == "TOPPLED") {
    return {ModelCondition::TOPPLED};
  } else if (value == "FRONTCRUSHED") {
    return {ModelCondition::FRONTCRUSHED};
  } else if (value == "BACKCRUSHED") {
    return {ModelCondition::BACKCRUSHED};
  } else if (value == "DAMAGED") {
    return {ModelCondition::DAMAGED};
  } else if (value == "REALLYDAMAGED") {
    return {ModelCondition::REALLYDAMAGED};
  } else if (value == "RUBBLE") {
    return {ModelCondition::RUBBLE};
  } else if (value == "SPECIAL_DAMAGED") {
    return {ModelCondition::SPECIAL_DAMAGED};
  } else if (value == "NIGHT") {
    return {ModelCondition::NIGHT};
  } else if (value == "SNOW") {
    return {ModelCondition::SNOW};
  } else if (value == "PARACHUTING") {
    return {ModelCondition::PARACHUTING};
  } else if (value == "GARRISONED") {
    return {ModelCondition::GARRISONED};
  } else if (value == "ENEMYNEAR") {
    return {ModelCondition::ENEMYNEAR};
  } else if (value == "WEAPONSET_VETERAN") {
    return {ModelCondition::WEAPONSET_VETERAN};
  } else if (value == "WEAPONSET_ELITE") {
    return {ModelCondition::WEAPONSET_ELITE};
  } else if (value == "WEAPONSET_HERO") {
    return {ModelCondition::WEAPONSET_HERO};
  } else if (value == "WEAPONSET_CRATEUPGRADE_ONE") {
    return {ModelCondition::WEAPONSET_CRATEUPGRADE_ONE};
  } else if (value == "WEAPONSET_CRATEUPGRADE_TWO") {
    return {ModelCondition::WEAPONSET_CRATEUPGRADE_TWO};
  } else if (value == "WEAPONSET_PLAYER_UPGRADE") {
    return {ModelCondition::WEAPONSET_PLAYER_UPGRADE};
  } else if (value == "DOOR_1_OPENING") {
    return {ModelCondition::DOOR_1_OPENING};
  } else if (value == "DOOR_1_CLOSING") {
    return {ModelCondition::DOOR_1_CLOSING};
  } else if (value == "DOOR_1_WAITING_OPEN") {
    return {ModelCondition::DOOR_1_WAITING_OPEN};
  } else if (value == "DOOR_1_WAITING_TO_CLOSE") {
    return {ModelCondition::DOOR_1_WAITING_TO_CLOSE};
  } else if (value == "DOOR_2_OPENING") {
    return {ModelCondition::DOOR_2_OPENING};
  } else if (value == "DOOR_2_CLOSING") {
    return {ModelCondition::DOOR_2_CLOSING};
  } else if (value == "DOOR_2_WAITING_OPEN") {
    return {ModelCondition::DOOR_2_WAITING_OPEN};
  } else if (value == "DOOR_2_WAITING_TO_CLOSE") {
    return {ModelCondition::DOOR_2_WAITING_TO_CLOSE};
  } else if (value == "DOOR_3_OPENING") {
    return {ModelCondition::DOOR_3_OPENING};
  } else if (value == "DOOR_3_CLOSING") {
    return {ModelCondition::DOOR_3_CLOSING};
  } else if (value == "DOOR_3_WAITING_OPEN") {
    return {ModelCondition::DOOR_3_WAITING_OPEN};
  } else if (value == "DOOR_3_WAITING_TO_CLOSE") {
    return {ModelCondition::DOOR_3_WAITING_TO_CLOSE};
  } else if (value == "DOOR_4_OPENING") {
    return {ModelCondition::DOOR_4_OPENING};
  } else if (value == "DOOR_4_CLOSING") {
    return {ModelCondition::DOOR_4_CLOSING};
  } else if (value == "DOOR_4_WAITING_OPEN") {
    return {ModelCondition::DOOR_4_WAITING_OPEN};
  } else if (value == "DOOR_4_WAITING_TO_CLOSE") {
    return {ModelCondition::DOOR_4_WAITING_TO_CLOSE};
  } else if (value == "ATTACKING") {
    return {ModelCondition::ATTACKING};
  } else if (value == "PREATTACK_A") {
    return {ModelCondition::PREATTACK_A};
  } else if (value == "FIRING_A") {
    return {ModelCondition::FIRING_A};
  } else if (value == "BETWEEN_FIRING_SHOTS_A") {
    return {ModelCondition::BETWEEN_FIRING_SHOTS_A};
  } else if (value == "RELOADING_A") {
    return {ModelCondition::RELOADING_A};
  } else if (value == "PREATTACK_B") {
    return {ModelCondition::PREATTACK_B};
  } else if (value == "FIRING_B") {
    return {ModelCondition::FIRING_B};
  } else if (value == "BETWEEN_FIRING_SHOTS_B") {
    return {ModelCondition::BETWEEN_FIRING_SHOTS_B};
  } else if (value == "RELOADING_B") {
    return {ModelCondition::RELOADING_B};
  } else if (value == "PREATTACK_C") {
    return {ModelCondition::PREATTACK_C};
  } else if (value == "FIRING_C") {
    return {ModelCondition::FIRING_C};
  } else if (value == "BETWEEN_FIRING_SHOTS_C") {
    return {ModelCondition::BETWEEN_FIRING_SHOTS_C};
  } else if (value == "RELOADING_C") {
    return {ModelCondition::RELOADING_C};
  } else if (value == "TURRET_ROTATE") {
    return {ModelCondition::TURRET_ROTATE};
  } else if (value == "POST_COLLAPSE") {
    return {ModelCondition::POST_COLLAPSE};
  } else if (value == "MOVING") {
    return {ModelCondition::MOVING};
  } else if (value == "DYING") {
    return {ModelCondition::DYING};
  } else if (value == "AWAITING_CONSTRUCTION") {
    return {ModelCondition::AWAITING_CONSTRUCTION};
  } else if (value == "PARTIALLY_CONSTRUCTED") {
    return {ModelCondition::PARTIALLY_CONSTRUCTED};
  } else if (value == "ACTIVELY_BEING_CONSTRUCTED") {
    return {ModelCondition::ACTIVELY_BEING_CONSTRUCTED};
  } else if (value == "PRONE") {
    return {ModelCondition::PRONE};
  } else if (value == "FREEFALL") {
    return {ModelCondition::FREEFALL};
  } else if (value == "ACTIVELY_CONSTRUCTING") {
    return {ModelCondition::ACTIVELY_CONSTRUCTING};
  } else if (value == "CONSTRUCTION_COMPLETE") {
    return {ModelCondition::CONSTRUCTION_COMPLETE};
  } else if (value == "RADAR_EXTENDING") {
    return {ModelCondition::RADAR_EXTENDING};
  } else if (value == "RADAR_UPGRADED") {
    return {ModelCondition::RADAR_UPGRADED};
  } else if (value == "PANICKING") {
    return {ModelCondition::PANICKING};
  } else if (value == "AFLAME") {
    return {ModelCondition::AFLAME};
  } else if (value == "SMOLDERING") {
    return {ModelCondition::SMOLDERING};
  } else if (value == "BURNED") {
    return {ModelCondition::BURNED};
  } else if (value == "DOCKING") {
    return {ModelCondition::DOCKING};
  } else if (value == "DOCKING_BEGINNING") {
    return {ModelCondition::DOCKING_BEGINNING};
  } else if (value == "DOCKING_ACTIVE") {
    return {ModelCondition::DOCKING_ACTIVE};
  } else if (value == "DOCKING_ENDING") {
    return {ModelCondition::DOCKING_ENDING};
  } else if (value == "CARRYING") {
    return {ModelCondition::CARRYING};
  } else if (value == "FLOODED") {
    return {ModelCondition::FLOODED};
  } else if (value == "LOADED") {
    return {ModelCondition::LOADED};
  } else if (value == "JETAFTERBURNER") {
    return {ModelCondition::JETAFTERBURNER};
  } else if (value == "JETEXHAUST") {
    return {ModelCondition::JETEXHAUST};
  } else if (value == "PACKING") {
    return {ModelCondition::PACKING};
  } else if (value == "UNPACKING") {
    return {ModelCondition::UNPACKING};
  } else if (value == "DEPLOYED") {
    return {ModelCondition::DEPLOYED};
  } else if (value == "OVER_WATER") {
    return {ModelCondition::OVER_WATER};
  } else if (value == "POWER_PLANT_UPGRADED") {
    return {ModelCondition::POWER_PLANT_UPGRADED};
  } else if (value == "CLIMBING") {
    return {ModelCondition::CLIMBING};
  } else if (value == "SOLD") {
    return {ModelCondition::SOLD};
  } else if (value == "RAPPELLING") {
    return {ModelCondition::RAPPELLING};
  } else if (value == "ARMED") {
    return {ModelCondition::ARMED};
  } else if (value == "POWER_PLANT_UPGRADING") {
    return {ModelCondition::POWER_PLANT_UPGRADING};
  } else if (value == "SPECIAL_CHEERING") {
    return {ModelCondition::SPECIAL_CHEERING};
  } else if (value == "CONTINUOUS_FIRE_SLOW") {
    return {ModelCondition::CONTINUOUS_FIRE_SLOW};
  } else if (value == "CONTINUOUS_FIRE_MEAN") {
    return {ModelCondition::CONTINUOUS_FIRE_MEAN};
  } else if (value == "CONTINUOUS_FIRE_FAST") {
    return {ModelCondition::CONTINUOUS_FIRE_FAST};
  } else if (value == "RAISING_FLAG") {
    return {ModelCondition::RAISING_FLAG};
  } else if (value == "CAPTURED") {
    return {ModelCondition::CAPTURED};
  } else if (value == "EXPLODED_FLAILING") {
    return {ModelCondition::EXPLODED_FLAILING};
  } else if (value == "EXPLODED_BOUNCING") {
    return {ModelCondition::EXPLODED_BOUNCING};
  } else if (value == "SPLATTED") {
    return {ModelCondition::SPLATTED};
  } else if (value == "USING_WEAPON_A") {
    return {ModelCondition::USING_WEAPON_A};
  } else if (value == "USING_WEAPON_B") {
    return {ModelCondition::USING_WEAPON_B};
  } else if (value == "USING_WEAPON_C") {
    return {ModelCondition::USING_WEAPON_C};
  } else if (value == "PREORDER") {
    return {ModelCondition::PREORDER};
  } else if (value == "CENTER_TO_LEFT") {
    return {ModelCondition::CENTER_TO_LEFT};
  } else if (value == "LEFT_TO_CENTER") {
    return {ModelCondition::LEFT_TO_CENTER};
  } else if (value == "CENTER_TO_RIGHT") {
    return {ModelCondition::CENTER_TO_RIGHT};
  } else if (value == "RIGHT_TO_CENTER") {
    return {ModelCondition::RIGHT_TO_CENTER};
  } else if (value == "RIDER1") {
    return {ModelCondition::RIDER1};
  } else if (value == "RIDER2") {
    return {ModelCondition::RIDER2};
  } else if (value == "RIDER3") {
    return {ModelCondition::RIDER3};
  } else if (value == "RIDER4") {
    return {ModelCondition::RIDER4};
  } else if (value == "RIDER5") {
    return {ModelCondition::RIDER5};
  } else if (value == "RIDER6") {
    return {ModelCondition::RIDER6};
  } else if (value == "RIDER7") {
    return {ModelCondition::RIDER7};
  } else if (value == "RIDER8") {
    return {ModelCondition::RIDER8};
  } else if (value == "STUNNED_FLAILING") {
    return {ModelCondition::STUNNED_FLAILING};
  } else if (value == "STUNNED") {
    return {ModelCondition::STUNNED};
  } else if (value == "SECOND_LIFE") {
    return {ModelCondition::SECOND_LIFE};
  } else if (value == "JAMMED") {
    return {ModelCondition::JAMMED};
  } else if (value == "ARMORSET_CRATEUPGRADE_ONE") {
    return {ModelCondition::ARMORSET_CRATEUPGRADE_ONE};
  } else if (value == "ARMORSET_CRATEUPGRADE_TWO") {
    return {ModelCondition::ARMORSET_CRATEUPGRADE_TWO};
  } else if (value == "USER_1") {
    return {ModelCondition::USER_1};
  } else if (value == "USER_2") {
    return {ModelCondition::USER_2};
  } else if (value == "DISGUISED") {
    return {ModelCondition::DISGUISED};
  } else {
    return {};
  }
}

std::optional<ModuleType> getModuleType(const std::string_view& value) {
  if (value == "ActiveBody") {
    return {ModuleType::ACTIVE_BODY};
  } else if (value == "AIUpdateInterface") {
    return {ModuleType::AI};
  } else if (value == "ArmorUpgrade") {
    return {ModuleType::ARMOR_UPGRADE};
  } else if (value == "AssistedTargetingUpdate") {
    return {ModuleType::ASSISTED_TARGETING};
  } else if (value == "AutoFindHealingUpdate") {
    return {ModuleType::AUTO_FIND_HEALING};
  } else if (value == "AutoHealBehavior") {
    return {ModuleType::AUTO_HEAL};
  } else if (value == "BaseRegenerateUpdate") {
    return {ModuleType::BASE_REGENERATE};
  } else if (value == "BattlePlanUpdate") {
    return {ModuleType::BATTLE_PLAN};
  } else if (value == "ChinookAIUpdate") {
    return {ModuleType::CHINOOK_AI};
  } else if (value == "CleanupAreaPower") {
    return {ModuleType::CLEANUP_AREA};
  } else if (value == "CleanupHazardUpdate") {
    return {ModuleType::CLEANUP_HAZARD};
  } else if (value == "CommandButtonHuntUpdate") {
    return {ModuleType::COMMAND_BUTTON_HUNT};
  } else if (value == "CountermeasuresBehavior") {
    return {ModuleType::COUNTERMEASURE};
  } else if (value == "CreateCrateDie") {
    return {ModuleType::CREATE_CRATE_DIE};
  } else if (value == "CreateObjectDie") {
    return {ModuleType::CREATE_OBJECT_DIE};
  } else if (value == "DefaultProductionExitUpdate") {
    return {ModuleType::DEFAULT_PRORDUCTION_EXIT};
  } else if (value == "DeletionUpdate") {
    return {ModuleType::DELETION};
  } else if (value == "DeliverPayloadAIUpdate") {
    return {ModuleType::DELIVER_PAYLOAD};
  } else if (value == "DeployStyleAIUpdate") {
    return {ModuleType::DEPLOY_STYLE_AI};
  } else if (value == "DestroyDie") {
    return {ModuleType::DESTROY_DIE};
  } else if (value == "DynamicShroudClearingRangeUpdate") {
    return {ModuleType::DYNAMIC_SHROUD_CLEARING_RANGE};
  } else if (value == "DozerAIUpdate") {
    return {ModuleType::DOZER_AI};
  } else if (value == "EjectPilotDie") {
    return {ModuleType::EJECT_PILOT_DIE};
  } else if (value == "ExperienceScalarUpgrade") {
    return {ModuleType::EXPERINCE_SCALAR_UPGRADE};
  } else if (value == "FireSpreadUpdate") {
    return {ModuleType::FIRE_SPREAD};
  } else if (value == "FireWeaponCollide") {
    return {ModuleType::FIRE_WEAPON_COLLISION};
  } else if (value == "FireWeaponUpdate") {
    return {ModuleType::FIRE_WEAPON};
  } else if (value == "FlammableUpdate") {
    return {ModuleType::FLAMMABLE};
  } else if (value == "FXListDie") {
    return {ModuleType::FX_LIST_DIE};
  } else if (value == "GarrisonContain") {
    return {ModuleType::GARRISON_CONTAIN};
  } else if (value == "GrantUpgradeCreate") {
    return {ModuleType::GRANT_UPGRADE};
  } else if (value == "GrantScienceUpgrade") {
    return {ModuleType::GRANT_SCIENCE_UPGRADE};
  } else if (value == "HealContain") {
    return {ModuleType::HEAL_CONTAIN};
  } else if (value == "HeightDieUpdate") {
    return {ModuleType::HEIGHT_DIE};
  } else if (value == "HelicopterSlowDeathBehavior") {
    return {ModuleType::HELICOPTER_SLOW_DEATH};
  } else if (value == "HelixContain") {
    return {ModuleType::HELIX_CONTAIN};
  } else if (value == "HighlanderBody") {
    return {ModuleType::HIGHLANDER_BODY};
  } else if (value == "HiveStructureBody") {
    return {ModuleType::HIVE_STRUCTURE_BODY};
  } else if (value == "ImmortalBody") {
    return {ModuleType::IMMORTAL_BODY};
  } else if (value == "InstantDeathBehavior") {
    return {ModuleType::INSTANT_DEATH};
  } else if (value == "JetAIUpdate") {
    return {ModuleType::JET_AI};
  } else if (value == "JetSlowDeathBehavior") {
    return {ModuleType::JET_SLOW_DEATH};
  } else if (value == "LaserUpdate") {
    return {ModuleType::LASER};
  } else if (value == "LifetimeUpdate") {
    return {ModuleType::LIFETIME};
  } else if (value == "LockWeaponCreate") {
    return {ModuleType::LOCK_WEAPON};
  } else if (value == "LocomotorSetUpgrade") {
    return {ModuleType::LOCOMOTOR_SET_UPGRADE};
  } else if (value == "MaxHealthUpgrade") {
    return {ModuleType::MAX_HEALTH_UPGRADE};
  } else if (value == "MissileAIUpdate") {
    return {ModuleType::MISSILE_AI};
  } else if (value == "ObjectCreationUpgrade") {
    return {ModuleType::OBJECT_CREATION_UPGRADE};
  } else if (value == "OCLUpdate") {
    return {ModuleType::OCL};
  } else if (value == "OCLSpecialPower") {
    return {ModuleType::OCL_SPECIAL_POWER};
  } else if (value == "OverlordContain") {
    return {ModuleType::OVERLORD_CONTAIN};
  } else if (value == "ParachuteContain") {
    return {ModuleType::PARACHUTE_CONTAIN};
  } else if (value == "ParkingPlaceBehavior") {
    return {ModuleType::PARKING_PLACE};
  } else if (value == "ParticleUplinkCannonUpdate") {
    return {ModuleType::PARTICLE_UPLINK_CANNON};
  } else if (value == "PhysicsBehavior") {
    return {ModuleType::PHYSICS};
  } else if (value == "PilotFindVehicleUpdate") {
    return {ModuleType::PILOT_FIND_VEHICLE};
  } else if (value == "PreorderCreate") {
    return {ModuleType::PREORDER_CREATE};
  } else if (value == "PointDefenseLaserUpdate") {
    return {ModuleType::POINT_DEFENSE_LASER};
  } else if (value == "PoisonedBehavior") {
    return {ModuleType::POISONED};
  } else if (value == "PowerPlantUpdate") {
    return {ModuleType::POWER_PLANT};
  } else if (value == "PowerPlantUpgrade") {
    return {ModuleType::POWER_PLANT_UPGRADE};
  } else if (value == "ProductionUpdate") {
    return {ModuleType::PRODUCTION};
  } else if (value == "RadarUpdate") {
    return {ModuleType::RADAR};
  } else if (value == "RadarUpgrade") {
    return {ModuleType::RADAR_UPGRADE};
  } else if (value == "RepairDockUpdate") {
    return {ModuleType::REPAIR_DOCK};
  } else if (value == "SlavedUpdate") {
    return {ModuleType::SLAVED};
  } else if (value == "SlowDeathBehavior") {
    return {ModuleType::SLOW_DEATH};
  } else if (value == "SpawnBehavior") {
    return {ModuleType::SPAWN};
  } else if (value == "SpecialAbility") {
    return {ModuleType::SPECIAL_POWER};
  } else if (value == "SpecialAbilityUpdate") {
    return {ModuleType::SPECIAL_POWER_UPDATE};
  } else if (value == "SpecialPowerCreate") {
    return {ModuleType::SPECIAL_POWER_CREATE};
  } else if (value == "SpectreGunshipUpdate") {
    return {ModuleType::SPECTRE_GUNSHIP};
  } else if (value == "SpectreGunshipDeploymentUpdate") {
    return {ModuleType::SPECTRE_GUNSHIP_DEPLOYMENT};
  } else if (value == "SpyVisionUpdate") {
    return {ModuleType::SPY_VISION};
  } else if (value == "SpyVisionSpecialPower") {
    return {ModuleType::SPY_VISION_SPECIAL_POWER};
  } else if (value == "SquishCollide") {
    return {ModuleType::SQUISH_COLLIDE};
  } else if (value == "StealthUpdate") {
    return {ModuleType::STEALTH};
  } else if (value == "StealthUpgrade") {
    return {ModuleType::STEALTH_UPGRADE};
  } else if (value == "StealthDetectorUpdate") {
    return {ModuleType::STEALTH_DETECTOR};
  } else if (value == "StructureBody") {
    return {ModuleType::STRUCTURE_BODY};
  } else if (value == "SupplyCenterCreate") {
    return {ModuleType::SUPPLY_CENTER};
  } else if (value == "SupplyCenterDockUpdate") {
    return {ModuleType::SUPPLY_CENTER_DOCK};
  } else if (value == "SupplyCenterProductionExitUpdate") {
    return {ModuleType::SUPPLY_CENTER_PRODUCTION_EXIT};
  } else if (value == "SwayClientUpdate") {
    return {ModuleType::SWAY_CLIENT};
  } else if (value == "ToppleUpdate") {
    return {ModuleType::TOPPLE};
  } else if (value == "TransitionDamageFX") {
    return {ModuleType::TRANSITION_DAMAGE_FX};
  } else if (value == "TransportAIUpdate") {
    return {ModuleType::TRANSPORT_AI};
  } else if (value == "TransportContain") {
    return {ModuleType::TRANSPORT_CONTAIN};
  } else if (value == "UnpauseSpecialPowerUpgrade") {
    return {ModuleType::UNPAUSE_SPECIAL_POWER_UPGRADE};
  } else if (value == "UpgradeDie") {
    return {ModuleType::UPGRADE_DIE};
  } else if (value == "VeterancyCrateCollide") {
    return {ModuleType::VETERANCY_CRATE_COLLISION};
  } else if (value == "VeterancyGainCreate") {
    return {ModuleType::VETERANCY_GAIN};
  } else if (value == "WeaponSetUpgrade") {
    return {ModuleType::WEAPON_SET_UPGRADE};
  } else {
    return {};
  }
}

std::optional<OCLLocation> getOCLLocation(const std::string_view& value) {
  if (value == "CREATE_AT_EDGE_NEAR_SOURCE") {
    return {OCLLocation::NEAR_SOURCE};
  } else if (value == "CREATE_AT_EDGE_NEAR_TARGET") {
    return {OCLLocation::NEAR_TARGET};
  } else if (value == "CREATE_AT_LOCATION") {
    return {OCLLocation::AT_LOCATION};
  } else if (value == "USE_OWNER_OBJECT") {
    return {OCLLocation::AT_OWNER};
  } else if (value == "CREATE_ABOVE_LOCATION") {
    return {OCLLocation::ABOVE_LOCATION};
  } else if (value == "CREATE_AT_EDGE_FARTHEST_FROM_TARGET") {
    return {OCLLocation::FARTHEST_FROM_TARGET};
  } else {
    return {};
  }
}

std::optional<RadarPriority> getRadarPriority(const std::string_view& value) {
  if (value == "INVALID") {
    return {RadarPriority::NONE};
  } else if (value == "NOT_ON_RADAR") {
    return {RadarPriority::NOT_ON_RADAR};
  } else if (value == "STRUCTURE") {
    return {RadarPriority::STRUCTURE};
  } else if (value == "UNIT") {
    return {RadarPriority::UNIT};
  } else if (value == "LOCAL_UNIT_ONLY") {
    return {RadarPriority::LOCAL_UNIT};
  } else {
    return {};
  }
}

std::optional<Shadow> getShadow(const std::string_view& value) {
  if (value == "NONE") {
    return {Shadow::NONE};
  } else if (value == "SHADOW_DECAL") {
    return {Shadow::DECAL};
  } else if (value == "SHADOW_VOLUME") {
    return {Shadow::VOLUME};
  } else if (value == "SHADOW_PROJECTION") {
    return {Shadow::PROJECTION};
  } else if (value == "SHADOW_DYNAMIC_PROJECTION") {
    return {Shadow::DYN_PROJECTION};
  } else if (value == "SHADOW_DIRECTIONAL_PROJECTION") {
    return {Shadow::DIRECTIONAL_PROJECTION};
  } else if (value == "SHADOW_ALPHA_DECAL") {
    return {Shadow::ALPHA_DECAL};
  } else if (value == "SHADOW_ADDITIVE_DECAL") {
    return {Shadow::ADDITIVE_DECAL};
  } else {
    return {};
  }
}

std::optional<SlowDeathPhase> getSlowDeathPhase(const std::string_view& value) {
  if (value == "INITIAL") {
    return {Objects::SlowDeathPhase::INITIAL};
  } else if (value == "MIDPOINT") {
    return {Objects::SlowDeathPhase::MIDPOINT};
  } else if (value == "FINAL") {
    return {Objects::SlowDeathPhase::FINAL};
  } else {
    return {};
  }
}

std::optional<Status> getStatus(const std::string_view& value) {
  if (value == "DESTROYED") {
    return {Status::DESTROYED};
  } else if (value == "CAN_ATTACK") {
    return {Status::CAN_ATTACK};
  } else if (value == "UNDER_CONSTRUCTION") {
    return {Status::UNDER_CONSTRUCTION};
  } else if (value == "UNSELECTABLE") {
    return {Status::UNSELECTABLE};
  } else if (value == "NO_COLLISIONS") {
    return {Status::NO_COLLISIONS};
  } else if (value == "NO_ATTACK") {
    return {Status::NO_ATTACK};
  } else if (value == "AIRBORNE_TARGET") {
    return {Status::AIRBORNE_TARGET};
  } else if (value == "PARACHUTING") {
    return {Status::PARACHUTING};
  } else if (value == "REPULSOR") {
    return {Status::REPULSOR};
  } else if (value == "HIJACKED") {
    return {Status::HIJACKED};
  } else if (value == "AFLAME") {
    return {Status::AFLAME};
  } else if (value == "BURNED") {
    return {Status::BURNED};
  } else if (value == "WET") {
    return {Status::WET};
  } else if (value == "IS_FIRING_WEAPON") {
    return {Status::IS_FIRING_WEAPON};
  } else if (value == "BRAKING") {
    return {Status::BRAKING};
  } else if (value == "STEALTHED") {
    return {Status::STEALTHED};
  } else if (value == "DETECTED") {
    return {Status::DETECTED};
  } else if (value == "CAN_STEALTH") {
    return {Status::CAN_STEALTH};
  } else if (value == "SOLD") {
    return {Status::SOLD};
  } else if (value == "UNDERGOING_REPAIR") {
    return {Status::UNDERGOING_REPAIR};
  } else if (value == "RECONSTRUCTING") {
    return {Status::RECONSTRUCTING};
  } else if (value == "MASKED") {
    return {Status::MASKED};
  } else if (value == "IS_ATTACKING") {
    return {Status::IS_ATTACKING};
  } else if (value == "IS_USING_ABILITY") {
    return {Status::IS_USING_ABILITY};
  } else if (value == "IS_AIMING_WEAPON") {
    return {Status::IS_AIMING_WEAPON};
  } else if (value == "NO_ATTACK_FROM_AI") {
    return {Status::NO_ATTACK_FROM_AI};
  } else if (value == "IGNORING_STEALTH") {
    return {Status::IGNORING_STEALTH};
  } else if (value == "IS_CARBOMB") {
    return {Status::IS_CARBOMB};
  } else if (value == "DECK_HEIGHT_OFFSET") {
    return {Status::DECK_HEIGHT_OFFSET};
  } else if (value == "RIDER1") {
    return {Status::RIDER1};
  } else if (value == "RIDER2") {
    return {Status::RIDER2};
  } else if (value == "RIDER3") {
    return {Status::RIDER3};
  } else if (value == "RIDER4") {
    return {Status::RIDER4};
  } else if (value == "RIDER5") {
    return {Status::RIDER5};
  } else if (value == "RIDER6") {
    return {Status::RIDER6};
  } else if (value == "RIDER7") {
    return {Status::RIDER7};
  } else if (value == "RIDER8") {
    return {Status::RIDER8};
  } else if (value == "FAERIE_FIRE") {
    return {Status::FAERIE_FIRE};
  } else if (value == "MISSILE_KILLING_SELF") {
    return {Status::MISSILE_KILLING_SELF};
  } else if (value == "REASSIGN_PARKING") {
    return {Status::REASSIGN_PARKING};
  } else if (value == "BOOBY_TRAPPED") {
    return {Status::BOOBY_TRAPPED};
  } else if (value == "IMMOBILE") {
    return {Status::IMMOBILE};
  } else if (value == "DISGUISED") {
    return {Status::DISGUISED};
  } else if (value == "DEPLOYED") {
    return {Status::DEPLOYED};
  } else {
    return {};
  }
}

std::optional<StealthLevel> getStealthLevel(const std::string_view& value) {
  if (value == "ATTACKING") {
    return {StealthLevel::ATTACKING};
  } else if (value == "MOVING") {
    return {StealthLevel::MOVING};
  } else if (value == "USING_ABILITY") {
    return {StealthLevel::USING_ABILITY};
  } else if (value == "FIRING_PRIMARY") {
    return {StealthLevel::FIRING_PRIMARY};
  } else if (value == "FIRING_SECONDARY") {
    return {StealthLevel::FIRING_SECONDARY};
  } else if (value == "FIRING_TERTIARY") {
    return {StealthLevel::FIRING_TERTIARY};
  } else if (value == "NO_BLACK_MARKET") {
    return {StealthLevel::NO_BLACK_MARKET};
  } else if (value == "TAKING_DAMAGE") {
    return {StealthLevel::TAKING_DAMAGE};
  } else if (value == "RIDERS_ATTACKING") {
    return {StealthLevel::RIDERS_ATTACKING};
  } else {
    return {};
  }
}

std::optional<Veterancy> getVeterancy(const std::string_view& value) {
  if (value == "REGULAR") {
    return {Veterancy::REGULAR};
  } else if (value == "VETERAN") {
    return {Veterancy::VETERAN};
  } else if (value == "ELITE") {
    return {Veterancy::ELITE};
  } else if (value == "HEROIC") {
    return {Veterancy::HEROIC};
  } else {
    return {};
  }
}

std::optional<WeaponSet::Condition> getWeaponSetCondition(const std::string_view& value) {
  if (value == "VETERAN") {
    return {WeaponSet::Condition::VETERAN};
  } else if (value == "ELITE") {
    return {WeaponSet::Condition::ELITE};
  } else if (value == "HERO") {
    return {WeaponSet::Condition::HERO};
  } else if (value == "PLAYER_UPGRADE") {
    return {WeaponSet::Condition::PLAYER_UPGRADE};
  } else if (value == "CRATE_UPGRADE_ONE") {
    return {WeaponSet::Condition::CRATE_UPGRADE_ONE};
  } else if (value == "CRATE_UPGRADE_TWO") {
    return {WeaponSet::Condition::CRATE_UPGRADE_TWO};
  } else if (value == "HIJACK") {
    return {WeaponSet::Condition::HIJACK};
  } else if (value == "CAR_BOMB") {
    return {WeaponSet::Condition::CAR_BOMB};
  } else if (value == "MONE_CLEARNING") {
    return {WeaponSet::Condition::MONE_CLEARNING};
  } else if (value == "RIDER1") {
    return {WeaponSet::Condition::RIDER1};
  } else if (value == "RIDER2") {
    return {WeaponSet::Condition::RIDER2};
  } else if (value == "RIDER3") {
    return {WeaponSet::Condition::RIDER3};
  } else if (value == "RIDER4") {
    return {WeaponSet::Condition::RIDER4};
  } else if (value == "RIDER5") {
    return {WeaponSet::Condition::RIDER5};
  } else if (value == "RIDER6") {
    return {WeaponSet::Condition::RIDER6};
  } else if (value == "RIDER7") {
    return {WeaponSet::Condition::RIDER7};
  } else if (value == "RIDER8") {
    return {WeaponSet::Condition::RIDER8};
  } else {
    return {};
  }
}

std::optional<WeaponSlot> getWeaponSlot(const std::string_view& value) {
  if (value == "PRIMARY") {
    return {Objects::WeaponSlot::PRIMARY};
  } else if (value == "SECONDARY") {
    return {Objects::WeaponSlot::SECONDARY};
  } else if (value == "TERTIARY") {
    return {Objects::WeaponSlot::TERTIARY};
  } else {
    return {};
  }
}

}
