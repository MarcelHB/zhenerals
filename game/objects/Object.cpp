#include "Object.h"

namespace ZH::Objects {

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

std::optional<ModuleType> getModuleType(const std::string_view& value) {
  if (value == "") {
    return {};
  } else if (value == "FireSpreadUpdate") {
    return {ModuleType::FIRE_SPREAD};
  } else if (value == "FireWeaponCollide") {
    return {ModuleType::FIRE_WEAPON_COLLISION};
  } else if (value == "FlammableUpdate") {
    return {ModuleType::FLAMMABLE};
  } else if (value == "HighlanderBody") {
    return {ModuleType::HIGHLANDER_BODY};
  } else if (value == "LifetimeUpdate") {
    return {ModuleType::LIFETIME};
  } else if (value == "PhysicsBehavior") {
    return {ModuleType::PHYSICS};
  } else if (value == "SlowDeathBehavior") {
    return {ModuleType::SLOW_DEATH};
  } else if (value == "SwayClientUpdate") {
    return {ModuleType::SWAY_CLIENT};
  } else if (value == "ToppleUpdate") {
    return {ModuleType::TOPPLE};
  } else {
    return {};
  }
}

std::optional<Shadow> getShadow(const std::string_view& value) {
  if (value == "SHADOW_DECAL") {
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


}
