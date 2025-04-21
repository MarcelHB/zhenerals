#include "Script.h"

namespace ZH::Script {

template<typename T>
std::optional<T> getScriptTypeByValue(uint32_t value) {
  if (value >= static_cast<std::underlying_type_t<T>>(T::COUNT)) {
    return {};
  }

  return {static_cast<T>(value)};
}

std::optional<ActionType> getScriptActionTypeByValue(uint32_t value) {
  return getScriptTypeByValue<ActionType>(value);
}

std::optional<ConditionType> getScriptConditionTypeByValue(uint32_t value) {
  return getScriptTypeByValue<ConditionType>(value);
}

std::optional<ParameterType> getScriptParameterTypeByValue(uint32_t value) {
  return getScriptTypeByValue<ParameterType>(value);
}

}
